# Unix Programs

These are a set of programs in C++ meant to recreate the functionality of common
unix programs. It serves both as a way to learn more C++, as well as better understand 
the programs I use everyday. 

The programs intentionally lack some of the functionality of the originals. 
They contain only the functionality that I use, which keeps the process more 
straight forward and easier to understand.

# 

The collection currently includes:

### Doug
A recreation of du. It prints the sizes of the subdirectories in a given path.
If no path specified, the current directory is used.
Symlinks are ignored, and the file size is human readable.
The sort flag (-s) sorts output by decreasing size, otherwise it is case-insensitive alphabetical.
The help flag (-h) shows usage.

### Ellis
A recreation of ls. By default it prints with the ls flags -hlN.
The hidden flag (-a) prints hidden files and directories.
The simple flag (-s) prints a simplified output and formatting.
The help flag (-h) shows usage.
The path can be specified, or if blank will be the current directory.

### Kat
A recreation of cat. It takes one or more files and concatenates their content to standard output.
The help flag (-h) shows usage.

### Trey
A recreation of tree. 
It displays the directory structure of a given path, or if black will be the current directory.
The depth flag (-l) sets how many levels the structure will display, and is 1 by default.
The directory flag (-d) displays only directories.
The hidden flag (-a) prints hidden files and directories.
the help flag (-h) shows usage.
