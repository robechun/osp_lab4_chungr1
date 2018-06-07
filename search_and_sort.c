#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>


#define NUM_THREADS 4


//takes a file/dir as argument, recurses,
// stores file name into allFiles list
void recur_file_search(char *file);
void printList(char**);		// prints list
void extendList(char***);	// extends the list

// Merge sort algorithm functions
void merge(int, int, int);
void mergeHelper(int, int);
void *mergeSort(void*);

char *search_term;
char **allFiles;			// array of all files found
int count = 0;				// global counter for above list
int allFiles_cap = 2;		// A changing cap for the "vector" that is allFiles
int thread_count = 1;

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		printf("Usage: my_file_search <search_term> <dir>\n");
		printf("Performs recursive file search for files/dirs matching\
				<search_term> starting at <dir>\n");
		exit(1);
	}

	//open the top-level directory
	DIR *dir = opendir(argv[2]);

	//make sure top-level dir is openable (i.e., exists and is a directory)
	if(dir == NULL)
	{
		perror("opendir failed");
		exit(1);
	}

	search_term = argv[1];
	
	// Store all file names in allFiles array to sort later
	allFiles = malloc(sizeof(char*) * allFiles_cap);
	char *argv_2_cpy = strdup(argv[2]);	// needed to pass into recursive search
	recur_file_search(argv_2_cpy);		// go through all files 



	// ------- SORTING START ---------- //
	pthread_t threads[NUM_THREADS];			// Array of threads to use


	clock_t start = clock(), diff;

	// Start threads on merging with 4 threads
	for (int i = 0; i < NUM_THREADS; i++) 
		assert(pthread_create(&threads[i], NULL, mergeSort, (void*) NULL) == 0);

	// Wait for all threads to finish
	for (int i = 0; i < NUM_THREADS; i++)
		assert(pthread_join(threads[i], NULL) == 0);

	// After returning from the last merge, need to merge one last time to get
	// Correct output
    merge(0, 
		  (NUM_THREADS / 2 - 1) / 2,
		  NUM_THREADS / 2 - 1);
    merge(NUM_THREADS / 2, 
		  NUM_THREADS/2 + (NUM_THREADS-1-NUM_THREADS/2)/2, 
		  NUM_THREADS - 1);
    merge(0,
		  (NUM_THREADS - 1)/2,
		  NUM_THREADS - 1);

	diff = clock() - start;

	// Print out the sorted files
	printList(allFiles);

	// TIME TAKEN
	int msec = diff * 1000 / CLOCKS_PER_SEC;
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
void recur_file_search(char *file)
{
	//check if directory is actually a file
	DIR *d = opendir(file);

	// If we need to allocate more memory to make space for the new files
	// that might be added, extend the allFiles list.
	if (count == allFiles_cap) {
		extendList(&allFiles);
	}

	//printf("DEBUG:recursed--%s\n", file);
	//printf("count:%d\n", count);
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
		if (strstr(file, search_term) != NULL) {
			allFiles[count] = malloc(sizeof(char*));
			allFiles[count] = file; 
			count++;
		}

		//printf("DEBUG: After insert file_only\n");
		//printf("DEBUG:  INSERTED:%s\n", allFiles[count-1]);

		//printf("-------- START PRINT LIST --------------\n");
		//printList(allFiles);
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
	if (strstr(file, search_term) != NULL) {
		allFiles[count] = malloc(sizeof(char*));
		allFiles[count] = file_cpy_dir;
		count++;
	}

	//printf("DEBUG:  INSERTED:%s\n", allFiles[count-1]);

	//printf("-------- START PRINT LIST --------------\n");
	//printList(allFiles);
	//printf("------- END PRINT LIST -----------------\n");
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
		free(list[i]);
	}

	free(list);
}

// extendList is to resize the dynamic array passed in.
// In this case, we are increasing the size of a char **list by 2x.
// This allows for more items to be stored in the dynamic array
void extendList(char ***list) {

	allFiles_cap = allFiles_cap * 2;	// increase the cap by 2x


	char **temp = realloc(*list, sizeof(char*) * allFiles_cap);

	// Something went wrong with realloc, so exit
	if (!temp) {
		perror("Reallocating failed!\n");
		exit(1);
	}

	//free(list);
	*list = temp;
	
	//printList(list);
}

void *mergeSort(void *arg) {
	
	// Calculating what the low, mid, and highs are
	int low = thread_count * (NUM_THREADS / 4);
	int high = (thread_count + 1) * (NUM_THREADS / 4) - 1;
	int mid = low + (high - low) / 2;

	// Start merging 
	if (low < high) {
		mergeHelper(low, mid);
		mergeHelper(mid+1, high);
		merge(low, mid, high);
	}

	return NULL;
}

void mergeHelper(int low, int high) {
	int mid = low + (high - low) / 2;

	// start merging recursively
	if (low < high) {
		mergeHelper(low, mid);
		mergeHelper(mid+1, high);
		merge(low, mid, high);
	}

}

void merge(int low, int mid, int high) {
	
	// new array sizes
	int left_size = mid - low + 1;
	int right_size = high - mid;

	// new arrays for merging
	char **left = malloc(sizeof(char*) * left_size);
	char **right = malloc(sizeof(char*) * right_size);


	// copy over values to the new left array
	int i = 0;
	for (i = 0; i < left_size; i++)
		left[i] = allFiles[i + low];

	// copy over values ot the new right array
	int j = 0;
	for (j = 0; j < right_size; j++)
		right[j] = allFiles[j + mid + 1];


	i = j = 0;
	int k = low;

	
	// merge both sides
	while (i < left_size && j < right_size) {
        if (strcmp(left[i], right[j]) <= 0)
            allFiles[k++] = left[i++];
        else
            allFiles[k++] = right[j++];
    }
	
	// Take care of remaining things that might not have been merged above
    while (i < left_size) {
        allFiles[k++] = left[i++];
    }
    while (j < right_size) {
        allFiles[k++] = right[j++];
    }

}
		


	
// TODO: make correct comments and erase wrong comments
// TODO: erase debug messages
