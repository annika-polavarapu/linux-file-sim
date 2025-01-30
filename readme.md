# CSCI 2275 – Data Structures - Final Project
### Overview

FileSim is a file system simulator that mimics the functionality of a basic file system. It offers several file system operations similar to Linux file commands that can create, edit, list, move, and delete files and directories. The simulator is designed to handle file management tasks in a hierarchical file system, where each node in the tree represents either a file or a directory. Addiionally, a Hash Table is used to optimize the search functionality.

### Features

The FileSim simulator implements these commands:

1. quit - exit the program
2. touch - create a new file in the current directory
3. edit - modify the contents of an existing file
4. mkdir - create a new directory in the current directory
5. ls - list the contents of the current directory
6. tree - print the entire directory tree starting from the current directory
7. rm - remove a file or directory from the current directory
8. cd - change the current directory
9. mv - move or rename a file or directory
10. pwd - print the path from the root to the current directory
11. stat - print information about a file
12. cat - display the contents of a file
13. search - search for files or directories by name

### Data Structures

Inode: Represents files and directories in the system. Each Inode contains name, file data, directory flag, file size, creation time, modification time, parent node pointer, and child nodes. 

Hash Table: A hash table with collision resolution using open addressing and linear probing to store file Inodes, allowing for efficient file searching.

### Search Optimization

The simulator implements a hash tablee to store file names and their corresponding Inodes. The hash table has three different collision handling mechanisms including open addressing with linear probing, open addressing with quadratic probing, and chaining with linked lists. The testHash.cpp file runs a test that creates 100 directories each containing 1 file and tests the three different collision handling mechanisms, printing out the number of collisions. 

