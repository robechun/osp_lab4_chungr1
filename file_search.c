#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>

bool validStartingDirectory(char *dir);
void search(char *term, char *dirname);
void search_helper(char *term, char *path, DIR *dir);

int main(int argc, char *argv[])
{

	// Usage: file_search <search term> <starting directory>
	// Therefore, should ALWAYS have 3 arguments
	if (argc != 3) {
		fprintf(stderr, "Incorrect number of arguments\n");
		exit(1);
	}
	
	if (!validStartingDirectory(argv[2])) {
		fprintf(stderr, "Not a valid starting directory\n");
		exit(1);
	}

	// Actual search and printing
	search(argv[1], argv[2]);


	return 0;
}

// Check to see that the starting directory is formatted correctly
// Should always start with a '/' and never end with a '/'
bool validStartingDirectory(char *dir)
{
	if (dir[0] != '/' || dir[strlen(dir)-1] == '/')
		return false;

	return true;
}

// search() is a primer-function for the recursive search
// Parameters: char *term --> filename to match and look for
// 			   char *dirname --> the starting directory
// Returns: NONE, but should print out all the files/directories matching term
void search(char *term, char *dirname)
{
	// opendir opens the directory specified by arg
	// Returns a pointer to DIR. If fails to open, returns NULL ptr
	DIR *start = opendir(dirname);

	// Failed to open the starting directory if NULL ptr
	if (start == NULL) {
		fprintf(stderr, "Could not open starting directory\n");
		return;
	}

	// Recurse and find files with term
	search_helper(term, dirname, start);
}


// search_helper() is the recursive function to search()
// Recursion will be in form of dfs
// Parameters: char *term --> filename to match and look for
// 			   char *path --> path of the current dir
// 			   char *dirname --> the directory to look into
// Returns: NONE, but should print out all the files/directories matching term
void search_helper(char *term, char *path, DIR *dir)
{
	struct dirent *dp;
	char *found;
	DIR *nextDir;
	bool isDir = false;
	char *fName;

	while (dir)
	{
		// readdir reads an element of argument and progresses pointer to next
		// element.
		// Returns ptr to dirent struct if successful, NULL if end of stream
		if ((dp = readdir(dir)) != NULL) {
			fName = dp->d_name;
			char *tmpPath = malloc(sizeof(char) * (strlen(path) +
									strlen(fName)));
			strcpy(tmpPath, path);
			strcat(tmpPath, fName);

			// Check to see if "." or ".." If so, do not recurse
			if (strcmp(fName, ".") == 0 || strcmp(fName, "..") == 0) {
				;	// Do nothing
			}
			// Try opening the result from read
			else if ((nextDir = opendir(tmpPath)) != NULL) {
				printf("HERE!\n");
				search_helper(term, tmpPath, nextDir);
				isDir = true;
			}


			// Check to see if the term matches the file we read from readdir
			if ((found = strstr(fName, term)) != NULL) {
				printf("%s/%s", path, fName);

				// If it is a directory, mark it with a ":"
				if (isDir) {
					printf(":");
					isDir = false;
				}

				// Formatting
				printf("\n");
			}

			free(tmpPath);

		}
		// END of directory--close the directory we're reading and return from
		// recursion
		else {
			closedir(dir);
			return;
		}
	}

	printf("done with search_helper for now\n");

}

// TODO: timing how long it takes
// TODO: Actual recursion
// TODO: open/read directory errors? (MAX_OPEN) do we need to account?
// TODO: is '/' a valid starting directory??
