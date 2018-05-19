#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

bool validStartingDirectory(char *dir);

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

// TODO: timing how long it takes
// TODO: Actual recursion
// TODO: open/read directory errors? (MAX_OPEN) do we need to account?
