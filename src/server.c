#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "network_types.h"
#include "server.h"
#include "utils.h"

int B_listen_for_message(Message *message)
{
	struct sockaddr_in server;
	struct sockaddr_in client;
	memset(&server, 0, sizeof(struct sockaddr_in));
	memset(&client, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(atoi(get_port()));

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		fprintf(stderr, "Error: could not create socket: %s %i\n", __FILE__, __LINE__);
		return -1;
	}

	if ((bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) < 0)
	{
		fprintf(stderr, "Error: could not bind socket: %s %i\n", __FILE__, __LINE__);
		return -1;
	}

	unsigned int address_len = 0;
	void *buf = malloc(MAX_BUFFER);
	memset(buf, 0, MAX_BUFFER);
	if ((recvfrom(sockfd, buf, MAX_BUFFER, 0, (struct sockaddr *)&client, &address_len)) < 0)
	{
		fprintf(stderr, "Error receiving message: %s %i\n", __FILE__, __LINE__);
		return -1;
	}

	memset(message, 0, sizeof(Message));
	message->data_len = MAX_BUFFER;
	message->from = client;
	message->type = 0;
	message->data = buf;
	close(sockfd);
	return 0;
}

