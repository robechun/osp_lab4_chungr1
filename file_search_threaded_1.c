#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#define NUM_THREADS 4

//takes a file/dir as argument, recurses,
// prints name if empty dir or not a dir (leaves)
void *recur_file_search(void *arg);

//share search term globally (rather than passing recursively)
const char *search_term;

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		printf("Usage: my_file_search <search_term> <dir>\n");
		printf("Performs recursive file search for files/dirs matching\
				<search_term> starting at <dir>\n");
		exit(1);
	}

	//don't need to bother checking if term/directory are swapped, since we can't
	// know for sure which is which anyway
	search_term = argv[1];

	char *file = strdup(argv[2]);
	//open the top-level directory
	DIR *dir = opendir(file);

	//make sure top-level dir is openable (i.e., exists and is a directory)
	if(dir == NULL)
	{
		perror("opendir failed");
		exit(1);
	}


    pthread_t threads[NUM_THREADS]; // array of threads to use
	char *toFree[NUM_THREADS];		// dynamic file name free thread-safe
    int i = 0;

    // ******  TIMING START  ************ //
    clock_t start = clock(), diff;

    //we have a directory, not a file, so check if its name
	// matches the search term
	if(strstr(argv[2], search_term) != NULL)
		printf("%s/\n", argv[2]);


    // LOOK IN THE STARTING DIRECTORY AND RECURSE INTO DIRECTORIES THAT ARE
    // ABLE TO BE RECURSED
    struct dirent *cur_file;
	while((cur_file = readdir(dir)) != NULL)
	{
		//make sure we don't recurse on . or ..
		if(strcmp(cur_file->d_name, "..") != 0 &&\
				strcmp(cur_file->d_name, ".") != 0)
		{
			//we need to pass a full path to the recursive function, 
			// so here we append the discovered filename (cur_file->d_name)
			// to the current path (file -- we know file is a directory at
			// this point)
			char *next_file_str = malloc(sizeof(char) * \
					strlen(cur_file->d_name) + \
					strlen(file) + 2);

			strncpy(next_file_str, file, strlen(file));
			strncpy(next_file_str + strlen(file), \
					"/", 1);
			strncpy(next_file_str + strlen(file) + 1, \
					cur_file->d_name, \
					strlen(cur_file->d_name) + 1);

			//printf("DEBUG: next_file_str: %s\n", next_file_str);
			//recurse on the file -- with threads!
			char *fileToPass = strdup(next_file_str);
            assert(pthread_create(&threads[i], NULL, recur_file_search,
							(void *) fileToPass) == 0);
			toFree[i] = fileToPass;
            i++;

			//free the dynamically-allocated string
			free(next_file_str);
		}
        
        if (i == NUM_THREADS) {
			//printf("DEBUG: before join\n");
            for (int j = 0; j < NUM_THREADS; j++) {
                assert(pthread_join(threads[j], NULL) == 0);
				free(toFree[j]);
				//printf("DEBUG: JOINED %d\n", j);
            }
            i = 0;
        }
            
	}
    diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
    // ******  TIMING END  ********** //

	//close the directory, or we will have too many files opened (bad times)
	closedir(dir);
	free(file);
    
    
    printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);

	return 0;
}


//This function takes a path to recurse on, searching for mathes to the
// (global) search_term.  The base case for recursion is when *file is
// not a directory.
//Parameters: the starting path for recursion (char *), which could be a
// directory or a regular file (or something else, but we don't need to
// worry about anything else for this assignment).
//Returns: nothing
//Effects: prints the filename if the base case is reached *and* search_term
// is found in the filename; otherwise, prints the directory name if the directory
// matches search_term.
void *recur_file_search(void *arg)
{
	char *file = (char*) arg;
	//check if directory is actually a file
	DIR *d = opendir(file);

	//NULL means not a directory (or another, unlikely error)
	if(d == NULL)
	{
		//opendir SHOULD error with ENOTDIR, but if it did something else,
		// we have a problem (e.g., forgot to close open files, got
		// EMFILE or ENFILE)
		if(errno != ENOTDIR)
		{	
			perror("Something weird happened!");
			fprintf(stderr, "While looking at: %s\n", file);
			exit(1);
		}

		//nothing weird happened, check if the file contains the search term
		// and if so print the file to the screen (with full path)
		if(strstr(file, search_term) != NULL)
			printf("%s\n", file);

		//no need to close d (we can't, it is NULL!)
		return NULL;
	}

	//we have a directory, not a file, so check if its name
	// matches the search term
	if(strstr(file, search_term) != NULL)
		printf("%s/\n", file);

	//call recur_file_search for each file in d
	//readdir "discovers" all the files in d, one by one and we
	// recurse on those until we run out (readdir will return NULL)
	struct dirent *cur_file;
	while((cur_file = readdir(d)) != NULL)
	{
		//make sure we don't recurse on . or ..
		if(strcmp(cur_file->d_name, "..") != 0 &&\
				strcmp(cur_file->d_name, ".") != 0)
		{
			//we need to pass a full path to the recursive function, 
			// so here we append the discovered filename (cur_file->d_name)
			// to the current path (file -- we know file is a directory at
			// this point)
			char *next_file_str = malloc(sizeof(char) * \
					strlen(cur_file->d_name) + \
					strlen(file) + 2);

			strncpy(next_file_str, file, strlen(file));
			strncpy(next_file_str + strlen(file), \
					"/", 1);
			strncpy(next_file_str + strlen(file) + 1, \
					cur_file->d_name, \
					strlen(cur_file->d_name) + 1);

			//recurse on the file
			recur_file_search(next_file_str);

			//free the dynamically-allocated string
			free(next_file_str);
		}
	}

	//close the directory, or we will have too many files opened (bad times)
	closedir(d);

	return NULL;
}
