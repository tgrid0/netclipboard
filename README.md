# netclipboard

## About

This is the simple network clipboard.

## Depency list

* pthread
* winsock (for Windows)
* xcb (for Linux)

## Building

There is no makefile at the moment.

Linux

To compile on Linux run these commands:

~~~~~
git clone https://github.com/tgridzero/netclipboard
cd netclipboard
gcc -std=c99 src/main.c src/network.c src/util.c src/lib/libclipboard/* -Iinclude/ -lpthread -lxcb
~~~~~

Windows

Clone project via git or download archive.
Open it in your favorite IDE, open build settings, add 'include' directory to includes, and 'wsock32' to libraries.

## Used 3rd-party

* libclipboard (https://github.com/jtanx/libclipboard)