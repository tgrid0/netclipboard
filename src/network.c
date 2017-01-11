#include "netclipboard.h"

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

void *receiver_thread_func()
{
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
