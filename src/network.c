#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include "network.h"
#include "utils.h"

void print_address(struct sockaddr sockaddr, const char *additional)
{
	char address_string[INET_ADDRSTRLEN] = {0};
	struct sockaddr_in *address = (struct sockaddr_in *)&sockaddr;
	inet_ntop(AF_INET, &(address->sin_addr), address_string, INET_ADDRSTRLEN);
	fprintf(stderr, "%s: %s\n", additional, address_string);
	struct sockaddr_in in_addr = *(struct sockaddr_in *)&sockaddr;
	fprintf(stderr, "Port %u\n--------------------------------------\n\n", ntohs(in_addr.sin_port));
}


int _B_listen_for_message(B_Connection connection, B_Message *message, unsigned int flags, char *file, int line)
{
	size_t message_size = MAX_MESSAGE_SIZE + sizeof(int) + sizeof("\0EM");
	void *data = malloc(message_size);
	memset(data, 0, message_size);
	unsigned int recv_flags = 0;
	message->from_addr_len = sizeof(struct sockaddr);
	if (flags & NON_BLOCKING)
	{
		recv_flags |= MSG_DONTWAIT;
	}
	int error = 0;
	if ((error = recvfrom(connection.sockfd, data, message_size, recv_flags, &(message->from_addr), &message->from_addr_len)) < 0)
	{
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
		{
			return 0;
		}
		fprintf(stderr, "B_listen_for_message recvfrom %s line %i error: %s\n", file, line, strerror(errno));
		return  -1;
	}

	memcpy(&message->type, (int *)data, sizeof(int));
	uint8_t *data_start = (uint8_t *)((uint8_t *)data + sizeof(int));
	uint8_t *data_end = (uint8_t *)memmem(data_start, message_size-sizeof(int), "\0EM", sizeof("\0EM"));
	if (data_end == NULL)
	{
		fprintf(stderr, "B_listen_for_message warning: Message does not contain proper footer\n");
		data_end = (uint8_t *)data + MAX_MESSAGE_SIZE;
	}
	size_t data_len = data_end - data_start;
	message->data = malloc(data_len+1);
	message->data_len = data_len+1;
	memset(message->data, 0, data_len+1);

	memcpy(message->data, data_start, data_len+1);
	uint8_t *data_bytes = (uint8_t *)message->data;
	data_bytes[data_len] = 0;
	message->from_name = 0;
	message->from_name_len = 0;
	BG_FREE(data);
	return 1;
}

void *construct_message(int type, void *data, size_t data_len, size_t *new_data_len)
{
	if ((data == NULL) || (data_len == 0))
	{
		size_t compiled_message_size = sizeof("\0EM") + sizeof(int);
		uint8_t *compiled_message = (uint8_t *)malloc(compiled_message_size);
		memset(compiled_message, 0, compiled_message_size);

		memcpy(compiled_message, &type, sizeof(int));
		uint8_t *compiled_message_footer = compiled_message+sizeof(int);
		memcpy(compiled_message_footer, "\0EM", sizeof("\0EM"));
		*new_data_len = compiled_message_size;
		return (void *)compiled_message;
	}
	void *compiled_message = NULL;
	size_t compiled_message_size = data_len + sizeof("\0EM") + sizeof(int);
	compiled_message = malloc(compiled_message_size);
	memset(compiled_message, 0, compiled_message_size);

	memcpy(compiled_message, &type, sizeof(int));
	uint8_t *data_start = (uint8_t *)((uint8_t *)compiled_message + sizeof(int));
	memcpy(data_start, data, data_len);
	uint8_t *data_end = (uint8_t *)(data_start + data_len);
	memcpy(data_end, "\0EM", sizeof("\0EM"));
	*new_data_len = compiled_message_size;
	return compiled_message;
}

int _B_send_message(B_Connection connection, int type, void *data, size_t data_len, char *file, int line)
{
	size_t new_data_len = 0;
	void *compiled_message = construct_message(type, data, data_len, &new_data_len);
	if ((sendto(connection.sockfd, compiled_message, new_data_len, 0, &connection.address, connection.address_len)) < 0)
	{
		fprintf(stderr, "B_send_message sendto %s line %i error: %s\n", file, line, strerror(errno));
		return -1;
	}

	BG_FREE(compiled_message);
	return 0;
}

int _B_send_to_address(B_Connection connection, B_Address address, int type, void *data, size_t data_len, char *file, int line)
{
	size_t new_data_len = 0;
	void *compiled_message = construct_message(type, data, data_len, &new_data_len);
	if ((sendto(connection.sockfd, compiled_message, new_data_len, 0, &address.address, address.address_len)) < 0)
	{
		fprintf(stderr, "B_send_to_addr sendto %s line %i error: %s\n", file, line, strerror(errno));
		return -1;
	}

	BG_FREE(compiled_message);
	return 0;
}

B_Address B_get_address_from_message(B_Message message)
{
	B_Address address;
	memset(&address, 0, sizeof(B_Address));
	address.address = message.from_addr;
	address.address_len = message.from_addr_len;
	return address;
}


int _B_send_reply(B_Connection connection, B_Message *message, int type, void *data, size_t data_len, char *file, int line)
{
	size_t new_data_len = 0;
	void *compiled_message = construct_message(type, data, data_len, &new_data_len);
	if ((sendto(connection.sockfd, compiled_message, new_data_len, 0, &message->from_addr, message->from_addr_len)) < 0)
	{
		fprintf(stderr, "B_send_reply sendto %s line %i error: %s\n", file, line, strerror(errno));
		return -1;
	}

	BG_FREE(compiled_message);
	return 0;
}

B_Connection B_server_connection(const char *port)
{
	B_Connection connection;
	memset(&connection, -1, sizeof(B_Connection));
	int sockfd = -1;
 	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
 	{
		return connection;
	}
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) 
	{
		fprintf(stderr, "B_connect_to error setting sock options\n");
	}
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(port));
	if ((bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) < 0)
	{
		fprintf(stderr, "B_connect_to error: could not bind server address: %s", strerror(errno));
		close(sockfd);
		return connection;
	}
	connection.sockfd = sockfd;
	connection.address = *(struct sockaddr *)&server_addr;
	connection.address_len = sizeof(struct sockaddr);
	connection.address_info = NULL;
	return connection;
	
}

B_Connection _B_connect_to(const char *server_addr, const char *port, unsigned int flags, char *file, int line)
{
	if (flags & SETUP_SERVER)
	{
		return B_server_connection(port);
	}
	B_Connection connection;
	memset(&connection, -1, sizeof(B_Connection));

	struct addrinfo *info;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	int error = 0;
	if ((error = getaddrinfo(server_addr, port, &hints, &info)) < 0)
	{
		fprintf(stderr, "B_connect_to error getaddrinfo %s line %i: %s\n", file, line, gai_strerror(error));
		return connection;
	}

	struct addrinfo *current = info;
	int sockfd = 0;
	for (current = info; current != NULL; current = current->ai_next)
	{
		if ((sockfd = socket(current->ai_family, current->ai_socktype, 0)) < 0)
		{
			continue;
		}
		break;
	}
	if (current == NULL)
	{
		fprintf(stderr, "B_Connection socket bind error %s line %i: %s\n", file, line, strerror(errno));
		return connection;
	}

	connection.address = *(struct sockaddr *)current->ai_addr;
	connection.address_len = current->ai_addrlen;
	connection.address_info = info;
	connection.sockfd = sockfd;
	return connection;
}

int B_get_sender_name(B_Message message, char *name, size_t name_len)
{
	char hostname[512] = {0};
	int error = 0;
	if ((error = getnameinfo(&message.from_addr, sizeof(struct sockaddr), hostname, 512, NULL, 0, NI_NOFQDN)) < 0)
	{
		fprintf(stderr, "B_get_sender_name error: %s\n", gai_strerror(error));
		return -1;
	}

	memcpy(hostname, name, name_len);
	return 0;
}

void B_close_connection(B_Connection connection)
{
	freeaddrinfo(connection.address_info);
	close(connection.sockfd);
}

void free_message(B_Message message)
{
	BG_FREE(message.data);
}
