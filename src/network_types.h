#ifndef __NETWORK_TYPES_H__
#define __NETWORK_TYPES_H__
#include <netinet/ip.h> 

#define MAX_BUFFER 2048
#define TEMP_SERVER_NAME "gregor-desktop"
#define TEMP_PORT "3940"

typedef struct
{
	unsigned int 		type;
	void			*data;
	size_t			data_len;
	//int			sockfd;
	struct sockaddr_in	from;
	//struct sockaddr_in 	to;
} Message;

char *get_port(void);
void free_message(Message message);
#endif
