#ifndef __NETWORK_H__
#define __NETWORK_H__
#include <sys/types.h>
#include <sys/socket.h>

#define B_connect_to(hostname, port, flags) _B_connect_to(hostname, port, flags, __FILE__, __LINE__)
#define B_listen_for_message(connection, message, flags) _B_listen_for_message(connection, message, flags, __FILE__, __LINE__)
#define B_send_message(connection, type, data, data_len) _B_send_message(connection, type, data, data_len,  __FILE__, __LINE__)
#define B_send_reply(connection, message, type, data, data_len) _B_send_message(connection, type, data, data_len, __FILE__, __LINE__);
#define MAX_PLAYERS 4
#define MAX_MESSAGE_SIZE 2048

enum CONNECT_TO_FLAGS	
{
	BORING_FLAG = 		0x00000000,
	SETUP_SERVER = 		0x00000001,
	CONNECT_TO_SERVER = 	0x00000002
};

enum LISTEN_FOR_MESSAGE_FLAGS
{
	BLOCKING,
	NON_BLOCKING
};

enum MESSAGE_TYPES
{
	BORING_TYPE,
	JOIN_REQUEST,
	COMMAND_STATE,
	ACTOR_STATE,
	ID_ASSIGNMENT,
	NEW_PLAYER,
	ACKNOWLEDGE_NP,
	NUM_MESSAGE_TYPES
};

typedef struct
{
	int			type;
	void			*data;
	ssize_t			data_len;
	struct sockaddr		from_addr;
	socklen_t		from_addr_len;
	char			*from_name;
	int			from_name_len;
} B_Message;

typedef struct
{
	struct sockaddr		address;
	struct addrinfo		*address_info;
	int			sockfd;
	ssize_t			address_len;
} B_Connection;

/* For one-player mode, set hostname to NULL for both the server and the client */
B_Connection _B_connect_to(const char *hostname, const char *port, unsigned int flags, char *file, int line);

/* Returns a message suitable for sending over the network. User is responsible for freeing the returned memory. */
void *construct_message(int type, void *data, size_t data_len, size_t *new_data_len);
int B_get_sender_name(B_Message message, char *name, size_t name_len);
int _B_send_message(B_Connection connection, int type, void *data, size_t data_len, char *file, int line);
int _B_listen_for_message(B_Connection connection, B_Message *message, unsigned int flags, char *file, int line);
int _B_send_reply(B_Connection connection, B_Message message, int type, void *data, size_t data_len, char *file, int line);
void B_close_connection(B_Connection connection);
void free_message(B_Message message);
#endif
