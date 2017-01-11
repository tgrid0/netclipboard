#include "netclipboard.h"

void usage()
{
	fprintf(stderr, "Usage:\nnetclipboard -s [server_address] port password\nnetclipboard -c [client_address] port server_address password");
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned short port_number;			/* Port number to use */
	int a1, a2, a3, a4;					/* Components of address in xxx.xxx.xxx.xxx form */
	int b1, b2, b3, b4;

	printf("argc %d\n", argc);
	if (argc > 2)
	{
		// client
		if ('c' == argv[1][1])
		{
			if (argc == 6)
			{
				if (sscanf(argv[3], "%u", &port_number) != 1 || sscanf(argv[4], "%d.%d.%d.%d", &b1, &b2, &b3, &b4) != 4 || sscanf(argv[5], "%s", &password) != 1)
				{
					usage();
				}
			}
			else if (argc == 7)
			{
				if (sscanf(argv[3], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4 || sscanf(argv[4], "%u", &port_number) != 1 || sscanf(argv[5], "%d.%d.%d.%d", &b1, &b2, &b3, &b4) != 4 || sscanf(argv[6], "%s", &password) != 1)
				{
					usage();
				}
			}
		}
		else if ('s' == argv[1][1])
		{
			if (argc == 4)
			{

				if (sscanf(argv[2], "%u", &port_number) != 1)
				{
					usage();
				}
			}
			else if (argc == 5)
			{
				if (sscanf(argv[2], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4)
				{
					usage();
				}
				if (sscanf(argv[3], "%u", &port_number) != 1)
				{
					usage();
				}
			}
			else
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
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	server.sin_family = AF_INET;
	server.sin_port = htons(port_number);

	/* Set address automatically if desired */
	if ('s' == argv[1][1])
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
	stop = 0;
	pthread_t network_thread;

	if(pthread_create(&network_thread, NULL, network_thread_func, (void*)NULL))
	{
		fprintf(stderr, "Error creating receiver_thread\n");
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
		fprintf(stderr, "Error joining receiver_thread\n");
		return 0;
	}

	closesocket(sd);
	sock_quit();

    return 0;
}
