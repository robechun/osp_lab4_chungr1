all: file_search

file_search: file_search.c
	gcc -Wall -o file_search file_search.c

clean:
	rm -f file_search
