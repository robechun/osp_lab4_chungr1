# osp_lab4_chungr1

## Usage
on the command line: <br>
```make clean``` <br>
```make all``` <br>
```./file_search <term to look for> <starting directory>``` <br>
* <term to look for> can contain spaces if it's enclosed in "", i.e "file name thing"
* Ensure <starting directory> is an absolute path.

## How it works
* The program will do a dfs search and look for all files that has ```<term to look for>``` in the file name (and directory)
* Starts at the ```<starting directory>```

## Threaded
* The threaded version uses 4 threads to scan the top-level directory and spawn threads that recurse on each of the files/directories found in that top-level directory.

## Extra Credit
* I implemented an "arraylist" to keep track of ALL files that it finds
* On top of the regular file_search (derived from Dr. Dingler's solution), it uses 4 threads to implement merge sort.
* Super fast!!


