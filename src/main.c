#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#ifdef _WIN32
	#include <winsock.h>
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>

	typedef int SOCKET;
#endif

#include "libclipboard.h"

int sock_init()
{
	#ifdef _WIN32
		WSADATA w;
		return (0 == WSAStartup(MAKEWORD(1,1), &w));
	#else
		return 0;
	#endif
}

int sock_quit()
{
	#ifdef _WIN32
		return WSACleanup();
	#else
		return 0;
	#endif
}

void usage()
{
	fprintf(stderr, "Usage: netclipboard server_address port\n");
	exit(0);
}

#define BUFFER_SIZE 4096

SOCKET sd;							/* Socket descriptor of server */
unsigned short port_number;			/* Port number to use */
int a1, a2, a3, a4;					/* Components of address in xxx.xxx.xxx.xxx form */
byte stop = 0;

struct sockaddr_in server;			/* Information about the server */
struct sockaddr_in client;			/* Information about the client */
struct hostent *hp;					/* Information about this computer */
char host_name[256];				/* Name of the server */

void *receiver_thread_func(){
	fd_set readfds;
	SOCKET n;
	char buffer[BUFFER_SIZE];
	int client_length;
	int res;
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(sd, &readfds);

	client_length = (int)sizeof(struct sockaddr_in);

	n = sd + 1;

	while (!stop)
	{
		res = select(n, &readfds, NULL, NULL, &timeout);
		if (res == -1) {
		    perror("select");
		} else if (res == 0) {
		    // printf("Timeout occurred!  No data after 10 seconds.\n");
			/* Continue silently */
		} else {
		    if (FD_ISSET(sd, &readfds)) {
		    	printf("Receiving...\n");
				/* Receive bytes from client */
				res = recvfrom(sd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client, &client_length);
				if (res < 0)
				{
					fprintf(stderr, "Could not receive datagram.\n");
				}
				printf("Message received: '%s'\n", buffer);
		    }

		}
	}
	return NULL;
}

static int get_line(char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return -1;

    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? -2 : 0;
    }

    buff[strlen(buff)-1] = '\0';
    return 0;
}

int main(int argc, char *argv[]) {
	/* Interpret command line */
	if (argc == 2)
	{
		/* Use local address */
		if (sscanf(argv[1], "%u", &port_number) != 1)
		{
			usage();
		}
	}
	else if (argc == 3)
	{
		/* Copy address */
		if (sscanf(argv[1], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4)
		{
			usage();
		}
		if (sscanf(argv[2], "%u", &port_number) != 1)
		{
			usage();
		}
	}
	else
	{
		usage();
	}

	/* Open windows connection */
	if (!sock_init())
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(0);
	}

	/* Open a datagram socket */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket.\n");
		sock_quit();
		exit(0);
	}

	/* Clear out server struct */
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	server.sin_family = AF_INET;
	server.sin_port = htons(port_number);

	/* Set address automatically if desired */
	if (argc == 2)
	{
		/* Get host name of this computer */
		gethostname(host_name, sizeof(host_name));
		hp = gethostbyname(host_name);

		/* Check for NULL pointer */
		if (hp == NULL)
		{
			fprintf(stderr, "Could not get host name.\n");
			closesocket(sd);
			sock_quit();
			exit(0);
		}

		/* Assign the address */
		server.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
		server.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
		server.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
		server.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
	}
	/* Otherwise assign it manually */
	else
	{
		server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
		server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
		server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
		server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;
	}

	/* Bind address to socket */
	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Could not bind name to socket.\n");
		closesocket(sd);
		sock_quit();
		exit(0);
	}

	/* Print out server information */
	printf("Server running on %u.%u.%u.%u:%u\n", (unsigned char)server.sin_addr.S_un.S_un_b.s_b1,
											  (unsigned char)server.sin_addr.S_un.S_un_b.s_b2,
											  (unsigned char)server.sin_addr.S_un.S_un_b.s_b3,
											  (unsigned char)server.sin_addr.S_un.S_un_b.s_b4,
											  port_number);
	printf("Type 'stop' to quit\n");

	char input_string[10];
	int res;
	pthread_t receiver_thread;

	if(pthread_create(&receiver_thread, NULL, receiver_thread_func, (void*)NULL))
	{
		fprintf(stderr, "Error creating thread\n");
		return 0;
	}

	while (!stop)
	{
		res = get_line("> ", input_string, sizeof(input_string));
		if (0 == res)
		{
			if (0 == strncmp("stop", input_string, strlen("stop")))
				stop = 1;
		}
	}

	if(pthread_join(receiver_thread, NULL))
	{
		fprintf(stderr, "Error joining thread\n");
		return 0;
	}

	closesocket(sd);
	sock_quit();

    return 0;
}
