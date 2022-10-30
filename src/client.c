#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include "network_types.h" 

int send_message(const char *server_name, void *message, size_t message_len)
{
	struct addrinfo *server_info = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	const char *port = get_port();
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if((getaddrinfo(server_name, port, &hints, &server_info)) < 0)
	{
		fprintf(stderr, "Could not get server address: %s %i\n", __FILE__, __LINE__);
		return -1;
	}

	int sockfd = -1;
	struct addrinfo *current;
	for (current = server_info; current != NULL; current = current->ai_next)
	{
		if ((sockfd = socket(current->ai_family, current->ai_socktype, current->ai_protocol)) < 0)
		{
			continue;
		}
		break;
	}
	if (sockfd < 0)
	{
		fprintf(stderr, "Error creating socket: %s %i\n", __FILE__, __LINE__);
		return -1;
	}

	//char message[] = "This is a test of the emergency broadcasting system.\n";
	if ((sendto(sockfd, message, message_len, 0, (struct sockaddr *)current->ai_addr, current->ai_addrlen)) < 0)
	{
		fprintf(stderr, "Error sending message to %s: %s %i\n", server_name, __FILE__, __LINE__);
		return -1;
	}
	freeaddrinfo(server_info);	
	close(sockfd);
	return 0;
}
