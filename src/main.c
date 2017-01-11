#include "netclipboard.h"

void usage()
{
	fprintf(stderr, "Usage:\nnetclipboard [host_address] host_port remote_address remote_port password");
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned short host_port, remote_port;			/* Port number to use */
	int a1, a2, a3, a4;								/* Components of address in xxx.xxx.xxx.xxx form */
	int b1, b2, b3, b4;

	if (argc > 4)
	{
		char format[32];
		snprintf(format, sizeof(format), "%%%ds", (int)(PASSWD_SIZE-1));
		if (argc == 5)
		{
			if (sscanf(argv[1], "%hu", &host_port) != 1 || sscanf(argv[2], "%d.%d.%d.%d", &b1, &b2, &b3, &b4) != 4
					|| sscanf(argv[3], "%hu", &remote_port) != 1 || sscanf(argv[4], format, &password) != 1)
			{
				usage();
			}
		}
		else if (argc == 6)
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

	/* Open a datagram socket */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket.\n");
		sock_quit();
		exit(0);
	}

	/* Clear out server struct */
	memset((void *)&host, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	host.sin_family = AF_INET;
	host.sin_port = htons(host_port);

	remote.sin_family = AF_INET;
	host.sin_port = htons(remote_port);

	remote.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)b1;
	remote.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)b2;
	remote.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)b3;
	remote.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)b4;

	/* Set address automatically if desired */
	if (argc == 5)
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
		host.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
		host.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
		host.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
		host.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
	}
	/* Otherwise assign it manually */
	else
	{
		host.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
		host.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
		host.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
		host.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;
	}

	/* Bind address to socket */
	if (bind(sd, (struct sockaddr *)&host, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Could not bind name to socket.\n");
		closesocket(sd);
		sock_quit();
		exit(0);
	}

	/* Print out server information */
	printf("Server running on %u.%u.%u.%u:%u.\nPassword is '%s'.\n", (unsigned char)host.sin_addr.S_un.S_un_b.s_b1,
											  (unsigned char)host.sin_addr.S_un.S_un_b.s_b2,
											  (unsigned char)host.sin_addr.S_un.S_un_b.s_b3,
											  (unsigned char)host.sin_addr.S_un.S_un_b.s_b4,
											  host_port, password);
	printf("Type 'stop' to quit.\n");

	char input_string[10];
	int res;
	stop = 0;
	pthread_t network_thread;

	if(pthread_create(&network_thread, NULL, network_thread_func, (void*)NULL))
	{
		fprintf(stderr, "Error creating receiver_thread.\n");
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

	if(pthread_join(network_thread, NULL))
	{
		fprintf(stderr, "Error joining receiver_thread.\n");
		return 0;
	}

	closesocket(sd);
	sock_quit();

    return 0;
}
