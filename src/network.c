#include "netclipboard.h"

int sock_init()
{
	#ifdef _WIN32
		WSADATA w;
		return (0 == WSAStartup(MAKEWORD(1,1), &w));
	#else
		return 1;
	#endif
}

int sock_quit()
{
	#ifdef _WIN32
		return WSACleanup();
	#else
		return 1;
	#endif
}

void sock_close(SOCKET s)
{
#ifdef _WIN32
	closesocket(s);
#else
	close(s);
#endif
}

void *network_thread_func()
{
	char curr_clip[BUFFER_SIZE] = "";
	fd_set readfds;
	SOCKET n;
	char buffer[BUFFER_SIZE];
	int client_length;
	int res;
	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	client_length = (int)sizeof(struct sockaddr_in);
	struct sockaddr_in tmp_remote;
	clipboard_c *cb = clipboard_new(NULL);
	if (cb == NULL) {
		printf("Clipboard initialization failed!\n");
		return NULL;
	}
	char received_once = 0;
	while (!stop)
	{
		FD_ZERO(&readfds);
		FD_SET(sd, &readfds);
		n = sd + 1;
		res = select(n, &readfds, NULL, NULL, &timeout);
		if (res == -1) {
		    perror("select");
		}
		else if (res == 0)
		{
			/* Continue silently */
		}
		else
		{
		    if (FD_ISSET(sd, &readfds)) {
		    	//printf("Receiving...\n");
				/* Receive bytes from client */
				res = recvfrom(sd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&tmp_remote, &client_length);
				if (res < 0)
				{
					fprintf(stderr, "Could not receive datagram.\n");
				}
				received_once = 1;
				//printf("Message received: '%s'\n", buffer);
				memset(&curr_clip[0], 0, sizeof(curr_clip));
				strncpy(curr_clip, buffer, strlen(buffer));
				curr_clip[strlen(curr_clip)] = '\0';
				clipboard_set_text_ex(cb, curr_clip, strlen(curr_clip), 0);
		    }
		}

		char *text = clipboard_text_ex(cb, NULL, 0);
		if (text != NULL)
		{
			if (strncmp(text, curr_clip, BUFFER_SIZE))
			{
				strncpy(curr_clip, text, BUFFER_SIZE);
				//printf("New clip: '%s'.\n", curr_clip);
				/* Send to client */
				if (received_once)
				{
					res = sendto(sd, curr_clip, BUFFER_SIZE, 0, (struct sockaddr*)&remote, sizeof(remote));
					if (res < 0)
					{
						fprintf(stderr, "Could not send datagram.\n");
					}
				}
			}
			free(text);
		}
	}

	clipboard_free(cb);
	return NULL;
}
