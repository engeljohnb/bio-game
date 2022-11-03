#ifndef __NETWORK_H__
#define __NETWORK_H__
#include <netinet/ip.h> 

#define MAX_BUFFER 2048
#define MAX_PLAYERS 4
#define TEMP_SERVER_NAME "gregor-desktop"
#define TEMP_PORT "6000"
#define BLOCKING 0
#define NON_BLOCKING 1

#define B_send_message(connection, type, message, len) _B_send_message(connection, type, message, len, __FILE__, __LINE__)
typedef struct 
{
	struct sockaddr		*address;
	size_t			address_len;
	int			sockfd;

} B_Connection;

enum MESSAGE_TYPES
{
	ERRORS_OR_SOMETHING = 0,
	JOIN_REQUEST,
	COMMAND_STATE,
	ACTOR_STATE,
	ID_ASSIGNMENT,
	NEW_PLAYER,
	ACKNOWLEDGE_NP
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
	size_t			from_name_len;
	//struct sockaddr_in 	to;
} Message;

char *get_port(void);
void free_message(Message message);
int B_listen_for_message(B_Connection connection, Message *message, unsigned int flags);
B_Connection B_connect_to(const char *recipient_name, const char *port);
int _B_send_message(B_Connection connection, unsigned int type, void *message, size_t message_len, char *file, int line);
B_Connection B_setup_recv_connection(char *port);
void B_close_connection(B_Connection connection);
#endif
