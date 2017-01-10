#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libclipboard.h>

#ifdef _WIN32
	#include <winsock.h>
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h> /* Needed for close() */

	typedef int SOCKET;
#endif

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
	fprintf(stderr, "Usage: netclipboard server_address port [client_address]\n");
	exit(0);
}

#define SIZE 512

int main(int argc, char *argv[]) {
	unsigned int port_number;				/* The port number to use */
	SOCKET sd;								/* The socket descriptor */
	int server_length;						/* Length of server struct */
	char send_buffer[SIZE] = "GET TIME\r\n";/* Data to send */
	time_t current_time;					/* Time received */
	struct hostent *hp;						/* Information about the server */
	struct sockaddr_in server;				/* Information about the server */
	struct sockaddr_in client;				/* Information about the client */
	int a1, a2, a3, a4;						/* Server address components in xxx.xxx.xxx.xxx form */
	int b1, b2, b3, b4;						/* Client address components in xxx.xxx.xxx.xxx form */
	char host_name[256];					/* Host name of this computer */

	if (argc != 3 && argc != 4)
	{
		usage();
	}
	if (sscanf(argv[1], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4)
	{
		usage();
	}
	if (sscanf(argv[2], "%u", &port_number) != 1)
	{
		usage();
	}
	if (argc == 4)
	{
		if (sscanf(argv[3], "%d.%d.%d.%d", &b1, &b2, &b3, &b4) != 4)
		{
			usage();
		}
	}

	if (!sock_init())
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		return 0;
	}

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket.\n");
		sock_quit();
		return 0;
	}

	/* Clear out server struct */
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	server.sin_family = AF_INET;
	server.sin_port = htons(port_number);

	/* Set server address */
	server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
	server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
	server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
	server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;

	/* Clear out client struct */
	memset((void *)&client, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	client.sin_family = AF_INET;
	client.sin_port = htons(0);

	if (argc == 3)
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
			return 0;
		}

		/* Assign the address */
		client.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
		client.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
		client.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
		client.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
	}
	else
	{
		client.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)b1;
		client.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)b2;
		client.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)b3;
		client.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)b4;
	}

	/* Bind local address to socket */
	if (bind(sd, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Cannot bind address to socket.\n");
		closesocket(sd);
		sock_quit();
		return 0;
	}

	closesocket(sd);
	sock_quit();

    clipboard_c *cb = clipboard_new(NULL);
    if (cb == NULL) {
        printf("Clipboard initialisation failed!\n");
        return 1;
    }

	char *text = clipboard_text_ex(cb, NULL, 0);
	if (text != NULL) {
		printf("%s\n", text);
		free(text);
	}

    clipboard_free(cb);
    return 0;
}
