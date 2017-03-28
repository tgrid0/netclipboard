# netclipboard

## About

This is the simple network clipboard.

## Depency list

* pthread (for Linux)
* winsock (for Windows)
* xcb (for Linux)

## Building

To compile on Linux, run these commands:

~~~~~
git clone https://github.com/tgrid0/netclipboard
cd netclipboard
make all
~~~~~

On Windows you can use [MinGW-w64](https://sourceforge.net/projects/mingw-w64/):

~~~~
mingw32-make all
~~~~


## Used 3rd-party

* libclipboard (https://github.com/jtanx/libclipboard)