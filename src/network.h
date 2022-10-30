#ifndef __NETWORK_H__
#define __NETWORK_H__
#include <netinet/ip.h> 

#define MAX_BUFFER 2048
#define MAX_PLAYERS 4
#define TEMP_SERVER_NAME "gregor-desktop"
#define TEMP_PORT "4090"

enum MESSAGE_TYPES
{
	ERRORS_OR_SOMETHING =	0,
	JOIN_REQUEST =		0x00000001,
	COMMAND_STATE =		0x00000002,
	ID_ASSIGNMENT = 	0x00000004
};

typedef struct
{
	unsigned int 		type;
	void			*data;
	size_t			data_len;
	//int			sockfd;
	struct sockaddr_in	from;
	size_t			from_len;
	char			*from_name;
	//struct sockaddr_in 	to;
} Message;

char *get_port(void);
void free_message(Message message);
int B_listen_for_message(Message *message);
int B_send_message(const char *server_name, unsigned int type, void *message, size_t message_len);
#endif
