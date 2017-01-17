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
#define REFRESH_TIME_SEC 2

typedef struct thread_params
{
	SOCKET sd;
	struct sockaddr_in remote;
	char password[PASSWD_SIZE];
} thread_params_t;

char stop;

#ifdef _WIN32
void sleep(int t);
#endif
int get_line(char *prmpt, char *buff, size_t sz);

int sock_init();
int sock_open();
int sock_quit();
void sock_close();
void sockaddr_fill(struct sockaddr_in* sin, int sin_family, int port, int oct1, int oct2, int oct3, int oct4);

#ifdef _WIN32
DWORD WINAPI network_thread_func(LPVOID lpParam);
#else
void *network_thread_func(void* p);
#endif

#endif /* INCLUDE_NETCLIPBOARD_H_ */
