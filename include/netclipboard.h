#ifndef INCLUDE_NETCLIPBOARD_H_
#define INCLUDE_NETCLIPBOARD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include "libclipboard.h"

#ifdef _WIN32
	#include <winsock.h>
	#include <windows.h>
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>

	typedef int SOCKET;
#endif

#define BUFFER_SIZE 4096

pthread_mutex_t curr_clip_lock;
static char curr_clip[BUFFER_SIZE] = "";

SOCKET sd;							/* Socket descriptor of server */
static byte stop = 0;

struct sockaddr_in server;			/* Information about the server */
struct sockaddr_in client;			/* Information about the client */
struct hostent *hp;					/* Information about this computer */
char host_name[256];				/* Name of the server */

#ifdef _WIN32
void sleep(int t);
#endif
int get_line(char *prmpt, char *buff, size_t sz);

int sock_init();

int sock_quit();

void *receiver_thread_func();

#endif /* INCLUDE_NETCLIPBOARD_H_ */
