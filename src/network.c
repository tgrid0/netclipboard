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

int sock_open(SOCKET* s)
{
	*s = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef _WIN32
	if (*s == INVALID_SOCKET)
	{
		return 0;
	}
#else
	if (*s < 0)
	{
		return 0;
	}
#endif
	return 1;
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

void sockaddr_fill(struct sockaddr_in* sin, int sin_family, int port, int oct1, int oct2, int oct3, int oct4)
{
	/* Set family and port */
	sin->sin_family = sin_family;
	sin->sin_port = htons(port);

#ifdef _WIN32
	sin->sin_addr.s_net = (unsigned char)oct1;
	sin->sin_addr.s_host = (unsigned char)oct2;
	sin->sin_addr.s_lh = (unsigned char)oct3;
	sin->sin_addr.s_impno = (unsigned char)oct4;
#else
	char addr[32];
	sprintf(addr, "%d.%d.%d.%d", oct1, oct2, oct3, oct4);
	inet_aton(addr, &(remote->sin_addr));
#endif
}


#ifdef _WIN32
DWORD WINAPI network_thread_func(LPVOID p)
#else
void *network_thread_func(void* p)
#endif
{
	thread_params_t* lparams;
	lparams = (thread_params_t*)p;
	SOCKET sd = lparams->sd;
	struct sockaddr_in remote = lparams->remote;
	char password[PASSWD_SIZE];
	strncpy(password, lparams->password, PASSWD_SIZE);
	free(lparams);

	char curr_clip[BUFFER_SIZE] = "";
	fd_set readfds;
	SOCKET n;

	int client_length;
	int res;
	struct timeval timeout;
	timeout.tv_sec = REFRESH_TIME_SEC;
	timeout.tv_usec = 0;

	client_length = (int)sizeof(struct sockaddr_in);
	struct sockaddr_in tmp_remote;
	clipboard_c *cb = clipboard_new(NULL);
	if (cb == NULL) {
		printf("Clipboard initialization failed!\n");
#ifdef _WIN32
		return 0;
#else
		return NULL;
#endif
	}
	char buffer[BUFFER_SIZE + PASSWD_SIZE + 1];
	while (!stop)
	{
		FD_ZERO(&readfds);
		FD_SET(sd, &readfds);
		n = sd + 1;
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
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
				res = recvfrom(sd, buffer, BUFFER_SIZE + PASSWD_SIZE + 1, 0, (struct sockaddr *)&tmp_remote, &client_length);
				if (res < 0)
				{
					fprintf(stderr, "Could not receive datagram.\n");
				}
				//printf("Message received: '%s'\n", buffer);
				if (strncmp(buffer, password, PASSWD_SIZE))
				{
					memset(&curr_clip[0], 0, sizeof(curr_clip));
					strncpy(curr_clip, buffer + PASSWD_SIZE, strlen(buffer) - PASSWD_SIZE);
					curr_clip[strlen(curr_clip)] = '\0';
					clipboard_set_text_ex(cb, curr_clip, strlen(curr_clip), 0);
				}
			}
		}

		char *text = clipboard_text_ex(cb, NULL, 0);
		if (text != NULL)
		{
			if (strncmp(text, curr_clip, BUFFER_SIZE))
			{
				memset(&buffer[0], 0, BUFFER_SIZE + PASSWD_SIZE + 1);
				sprintf(buffer, "%s%s", password, curr_clip);
				//printf("New clip: '%s'.\n", curr_clip);
				/* Send to client */
				res = sendto(sd, buffer, BUFFER_SIZE + PASSWD_SIZE + 1, 0, (struct sockaddr*)&remote, sizeof(remote));
				if (res < 0)
				{
					fprintf(stderr, "Could not send datagram.\n");
				}
				//printf("Datagramm sent\n");
			}
			free(text);
		}
	}

	clipboard_free(cb);
#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}
