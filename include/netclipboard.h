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
	#include <netdb.h>
	#include <sys/select.h>		// need for fd_set type

	typedef int SOCKET;
#endif

#define BUFFER_SIZE 4096
#define PASSWD_SIZE 20

typedef struct clip_packet
{
	char password[PASSWD_SIZE];
	char clip[BUFFER_SIZE];
} clip_packet_t;

SOCKET sd;							/* Socket descriptor of server */
char stop;

struct sockaddr_in host;			/* Information about the server */
struct sockaddr_in remote;			/* Information about the client */
char host_name[256];				/* Name of the server */
char password[PASSWD_SIZE];			/* Sort of security */

#ifdef _WIN32
void sleep(int t);
#endif
int get_line(char *prmpt, char *buff, size_t sz);

int sock_init();

int sock_quit();

void sock_close();

#ifdef _WIN32
DWORD WINAPI network_thread_func();
#else
void *network_thread_func();
#endif

#endif /* INCLUDE_NETCLIPBOARD_H_ */
