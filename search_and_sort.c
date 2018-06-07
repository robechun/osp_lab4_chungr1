#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <errno.h>

//takes a file/dir as argument, recurses,
// prints name if empty dir or not a dir (leaves)
void recur_file_search(char *file);
void printList(char**);
void extendList(char**);

const char *search_term;    // share_term for global use instead of passing in
char **allFiles;			// array of all files found
int count = 0;				// global counter for above list
int allFiles_cap = 10;		// A changing cap for the "vector" that is allFiles

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

	//open the top-level directory
	DIR *dir = opendir(argv[2]);

	//make sure top-level dir is openable (i.e., exists and is a directory)
	if(dir == NULL)
	{
		perror("opendir failed");
		exit(1);
	}
	
	allFiles = malloc(sizeof(char*) * allFiles_cap);
	char *argv_2_cpy = strdup(argv[2]);
	clock_t start = clock(), diff;
	recur_file_search(argv_2_cpy);
	diff = clock() - start;

	int msec = diff * 1000 / CLOCKS_PER_SEC;

	printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);

	printList(allFiles);

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
void recur_file_search(char *file)
{
	//check if directory is actually a file
	DIR *d = opendir(file);

	// If we need to allocate more memory to make space for the new files
	// that might be added, extend the allFiles list.
	if (count == allFiles_cap) {
		extendList(allFiles);
	}

	//printf("DEBUG:recursed--%s\n", file);
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

		//nothing weird happened, so add to list of found paths
		//printf("DEBUG:_NOT a directory\n");
		allFiles[count] = malloc(sizeof(char*));
		allFiles[count] = file;
		count++;

		//printf("DEBUG: After insert file_only\n");
		//printf("DEBUG:  INSERTED:%s\n", allFiles[count-1]);

		//printf("-------- START PRINT LIST --------------\n");
		printList(allFiles);
		//printf("------- END PRINT LIST -----------------\n");
		//no need to close d (we can't, it is NULL!)
		return;
	}


	//printf("DEBUG:_IS a directory\n");
	//we have a directory, so add "/" onto the end and put it in the list
	char *file_cpy_dir = malloc(sizeof(char) * strlen(file) + 1);
	strncpy(file_cpy_dir, file, strlen(file));
	strncpy(file_cpy_dir + strlen(file), "/", 1);

	//printf("DEBUG: cpy is:%s\n", file_cpy_dir);

	allFiles[count] = malloc(sizeof(char*));
	allFiles[count] = file_cpy_dir;
	count++;

	//printf("DEBUG:  INSERTED:%s\n", allFiles[count-1]);

	printf("-------- START PRINT LIST --------------\n");
	printList(allFiles);
	printf("------- END PRINT LIST -----------------\n");
	//call recur_file_search for each file in d
	//readdir "discovers" all the files in d, one by one and we
	// recurse on those until we run out (readdir will return NULL)
	struct dirent *cur_file;
	while((cur_file = readdir(d)) != NULL)
	{
		//printf("Hello:%s\n",file);
		//printf("DEBUG: cur_file is:%s\n", cur_file->d_name);
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

			//printf("DEBUG: FILE is:%s\n", file);
			strncpy(next_file_str, file, strlen(file));
			strncpy(next_file_str + strlen(file), \
					"/", 1);
			strncpy(next_file_str + strlen(file) + 1, \
					cur_file->d_name, \
					strlen(cur_file->d_name) + 1);

			//printf("DEBUG: FILE is:%s\n", file);
			//printf("DEBUG: next_file is:%s\n", next_file_str);

			//recurse on the file
			recur_file_search(next_file_str);
			//printf("DEBUG: *** returned from recursion ***\n");
			//printf("hello2:%s\n", file);
			//free the dynamically-allocated string
			//free(next_file_str);
		}
	}
	//printf("DEBUG: ---- end of while loop ---\n");

	free(file);
	//close the directory, or we will have too many files opened (bad times)
	closedir(d);
}


// printList simply prints out the contents of the string array (char **) passed
void printList(char **list) {

	for (int i = 0; i < count; i++)
	{
		printf("%s\n", list[i]);
	}
}

// extendList is to resize the dynamic array passed in.
// In this case, we are increasing the size of a char **list by 2x.
// This allows for more items to be stored in the dynamic array
void extendList(char **list) {

	allFiles_cap = allFiles_cap * 2;	// increase the cap by 2x


	char **temp = realloc(list, sizeof(char*) * allFiles_cap);

	// Something went wrong with realloc, so exit
	if (!temp) {
		perror("Reallocating failed!\n");
		exit(1);
	}

	free(list);
	list = temp;
	
	printList(list);
}
