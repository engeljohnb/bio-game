#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "utils.h"
#include "network.h"

int B_send_message(const char *recipient_name, unsigned int type, void *message, size_t message_len)
{
	struct addrinfo *server_info = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	const char *port = get_port();
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	int error = 0;
	if((error = getaddrinfo(recipient_name, port, &hints, &server_info)) < 0)
	{
		fprintf(stderr, "Could not get server address: %s %i %i\n", __FILE__, __LINE__, error);
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
	freeaddrinfo(server_info);	

	size_t data_len = message_len + sizeof(unsigned int) + sizeof("\0EM");
	void *packaged_message = malloc(message_len);
	memset(packaged_message, 0, message_len);

	uint8_t *message_iter = (uint8_t *)packaged_message;
	memcpy(message_iter, &type, sizeof(unsigned int));
	message_iter += sizeof(unsigned int);
	memcpy(message_iter, message, message_len);
	message_iter += message_len;
	memcpy(message_iter, "\0EM", sizeof("\0EM"));

	if ((sendto(sockfd, packaged_message, data_len, 0, (struct sockaddr *)current->ai_addr, current->ai_addrlen)) < 0)
	{
		fprintf(stderr, "Error sending message to %s: %s %i\n", recipient_name, __FILE__, __LINE__);
		return -1;
	}
	close(sockfd);
	return 0;
}

int B_listen_for_message(Message *message)
{
	struct sockaddr_in *host = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	struct sockaddr_in recipient;
	struct addrinfo *host_info;
	struct addrinfo hints;
	memset(host, 0, sizeof(struct sockaddr_in));
	memset(&recipient, 0, sizeof(struct sockaddr_in));
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	/*server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(atoi(get_port()));*/
	if ((getaddrinfo(NULL, get_port(), &hints, &host_info)) < 0)
	{
		fprintf(stderr, "Could not get address info: %s %i\n", __FILE__, __LINE__);
		message->data = NULL;
		message->from_name = NULL;
		return -1;
	}

	struct addrinfo *current = host_info;
	int sockfd = -1;
	 
	for (current = host_info; current != NULL; current = current->ai_next)
	{

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd < 0)
		{
			fprintf(stderr, "Error: could not create socket: %s %i\n", __FILE__, __LINE__);
			continue;
		}
		int yes = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) 
		{
			fprintf(stderr, "Error setting socket options: %s %i\n", __FILE__, __LINE__);
		}
		if ((bind(sockfd, current->ai_addr, sizeof(struct sockaddr))) < 0)
		{
			fprintf(stderr, "Error: could not bind socket: %s %i %i\n", __FILE__, __LINE__, errno);
			close(sockfd);
			continue;
		}
		host = (struct sockaddr_in *)(current->ai_addr);
		break;
	}
	freeaddrinfo(host_info);
	if (current == NULL)
	{
		fprintf(stderr, "Error: Could not bind host address: %s %i\n", __FILE__, __LINE__);
		message->data = NULL;
		message->from_name = NULL;
		return -1;
	}

	unsigned int address_len = sizeof(struct sockaddr);
	int bytes_read = 0;
	void *buf = malloc(MAX_BUFFER);
	memset(buf, 0, MAX_BUFFER);
	if ((bytes_read = recvfrom(sockfd, buf, MAX_BUFFER, 0, (struct sockaddr *)&recipient, &address_len)) < 0)
	{
		fprintf(stderr, "Error receiving message: %s %i\n", __FILE__, __LINE__);
		message->data = NULL;
		message->from_name = NULL;
		return -1;
	}

	//recipient.sin_family = AF_INET;
	uint8_t *data_end = memmem(buf, MAX_BUFFER, "\0EM", 3);
	uint8_t *message_data = ((uint8_t *)buf + sizeof(unsigned int));
	if (data_end == NULL)
	{
		char data_string[128] = {0};
		memcpy(data_string, message_data, 128);
		data_string[127] = 0;
		fprintf(stderr, "Error: Improper message received: %s\n%s %u\n", data_string, __FILE__, __LINE__);
		message->data = NULL;
		message->from_name = NULL;
		return -1;
	}
	size_t data_len = data_end - message_data;
	memset(message, 0, sizeof(Message));
	message->data = (uint8_t *)malloc(data_len);
	memset(message->data, 0, data_len);
	memcpy(message->data, message_data, data_len);
	message->from_name = NULL;
	message->type = *(unsigned int *)buf;
	message->data_len = data_len;
	message->from_len = address_len;
	message->from = recipient;
	char recipient_name[256] = {0};
	char service_name[256] = {0};
	int error = 0;
	if ((error = getnameinfo((struct sockaddr*)&recipient, sizeof(struct sockaddr), recipient_name, 256, service_name, 256, NI_NOFQDN | NI_DGRAM | NI_NAMEREQD)) < 0)
	{
		fprintf(stderr, "Error getting recipient name: %s %i %s\n", __FILE__, __LINE__, gai_strerror(error));
		message->data = NULL;
		message->from_name = NULL;
		return -1;
	}
	message->from_name = malloc(256);
	memset(message->from_name, 0, 256);
	memcpy(message->from_name, recipient_name, strnlen(recipient_name, 256));
	BG_FREE(buf);
	close(sockfd);
	return 0;
}

char *get_port(void)
{
	return TEMP_PORT;
}

void free_message(Message message)
{
	BG_FREE(message.from_name);
	BG_FREE(message.data);
}
