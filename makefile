all: file_search file_search_threaded

file_search: file_search.c
	gcc -Wall -o file_search file_search.c

file_search_threaded: file_search_threaded.c
	gcc -Wall -o file_search_treaded file_search_threaded.c -pthread

clean:
	rm -f file_search file_search_threaded
