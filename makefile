all: file_search file_search_threaded_1 file_search_threaded_2 search_and_sort

file_search: file_search.c
	gcc -Wall -o file_search file_search.c

file_search_threaded_1: file_search_threaded_1.c
	gcc -Wall -o file_search_threaded_1 file_search_threaded_1.c -pthread

file_search_threaded_2: file_search_threaded_2.c
	gcc -Wall -o file_search_threaded_2 file_search_threaded_2.c -pthread

search_and_sort: search_and_sort.c
	gcc -Wall -o search_and_sort search_and_sort.c -pthread

clean:
	rm -f file_search file_search_threaded file_search_threaded_1
