#include "netclipboard.h"

void usage()
{
	fprintf(stderr, "Usage:\nnetclipboard [host_address] host_port remote_address remote_port password");
	exit(0);
}

int main(int argc, char *argv[])
{
	SOCKET sd;									/* Socket descriptor of server */
	struct sockaddr_in host;					/* Information about the server */
	struct sockaddr_in remote;					/* Information about the client */
	char host_name[256];						/* Name of the server */
	char password[PASSWD_SIZE];					/* Sort of security */
	unsigned short host_port, remote_port;		/* Port number to use */
	int a1, a2, a3, a4;							/* Components of address in xxx.xxx.xxx.xxx form */
	int b1, b2, b3, b4;

	if (argc > 3)
	{
		char format[32];
		snprintf(format, sizeof(format), "%%%ds", (int)(PASSWD_SIZE-1));
		if (argc == 4)
		{
			if (sscanf(argv[1], "%hu", &host_port) != 1 || sscanf(argv[2], "%d.%d.%d.%d", &b1, &b2, &b3, &b4) != 4
					|| sscanf(argv[3], "%hu", &remote_port) != 1 || sscanf(argv[4], format, &password) != 1)
			{
				usage();
			}
		}
		else if (argc == 5)
		{
			if (sscanf(argv[1], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4 || sscanf(argv[2], "%hu", &host_port) != 1
					|| sscanf(argv[3], "%d.%d.%d.%d", &b1, &b2, &b3, &b4) != 4 || sscanf(argv[4], "%hu", &remote_port) != 1
					|| sscanf(argv[5], format, &password) != 1)
			{
				usage();
			}
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

	if (!sock_open(&sd))
	{
		fprintf(stderr, "Could not create socket.\n");
		sock_quit();
		exit(0);
	}


	memset((void *)&host, '\0', sizeof(struct sockaddr_in));
	memset((void *)&remote, '\0', sizeof(struct sockaddr_in));

	sockaddr_fill(&remote, AF_INET, remote_port, b1, b2, b3, b4);
	/* Set address automatically if desired */
	if (argc == 5)
	{
		struct hostent *hp;
		/* Get host name of this computer */
		gethostname(host_name, sizeof(host_name));
		hp = gethostbyname(host_name);

		/* Check for NULL pointer */
		if (hp == NULL)
		{
			fprintf(stderr, "Could not get host name.\n");
			sock_close(sd);
			sock_quit();
			exit(0);
		}

		sockaddr_fill(&host, AF_INET, host_port, hp->h_addr_list[0][0], hp->h_addr_list[0][1], hp->h_addr_list[0][2], hp->h_addr_list[0][3]);
	}
	/* Otherwise assign it manually */
	else
	{
		sockaddr_fill(&host, AF_INET, host_port, a1, a2, a3, a4);
	}

	/* Bind address to socket */
	if (bind(sd, (struct sockaddr *)&host, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Could not bind name to socket.\n");
		sock_close(sd);
		sock_quit();
		exit(0);
	}
#ifdef _WIN32
	/* Print out server information */
	printf("Server running on %u.%u.%u.%u:%u.\nPassword is '%s'.\n", (unsigned char)host.sin_addr.s_net,
											  (unsigned char)host.sin_addr.s_host,
											  (unsigned char)host.sin_addr.s_lh,
											  (unsigned char)host.sin_addr.s_impno,
											  host_port, password);
#else
	char addr[32];
	inet_ntop(AF_INET, &(host.sin_addr), addr, 32);
	printf("Server running on %s:%u.\nPassword is '%s'.\n", addr, host_port, password);
#endif
	printf("Type 'stop' to quit.\n");

	int res;
	stop = 0;

	thread_params_t *params = (thread_params_t*)malloc(sizeof(thread_params_t));
	strncpy(params->password, password, PASSWD_SIZE);
	params->remote = remote;
	params->sd = sd;

#ifdef _WIN32
	HANDLE network_thread = CreateThread(NULL, 0, network_thread_func, params, 0, NULL);
	if (!network_thread)
	{
		fprintf(stderr, "Error creating network_thread.\n");
		return 0;
	}
#else
	pthread_t network_thread;

	if(pthread_create(&network_thread, NULL, network_thread_func, (void*)params))
	{
		fprintf(stderr, "Error creating network_thread.\n");
		return 0;
	}
#endif
	char input_string[10];
	while (!stop)
	{
		res = get_line("> ", input_string, sizeof(input_string));
		if (0 == res)
		{
			if (0 == strncmp("stop", input_string, strlen("stop")))
				stop = 1;
		}
	}
#ifdef _WIN32
	DWORD retval;
	retval = WaitForSingleObject(network_thread, (REFRESH_TIME_SEC + 1) * 1000);
	if (WAIT_FAILED == retval || WAIT_TIMEOUT == retval)
	{
		fprintf(stderr, "Error waiting network_thread.\n");
		return 0;
	}
#else
	if(pthread_join(network_thread, NULL))
	{
		fprintf(stderr, "Error joining network_thread.\n");
		return 0;
	}
#endif
	sock_close(sd);
	sock_quit();

    return 0;
}
