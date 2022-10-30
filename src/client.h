#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "network_types.h"


int send_message(const char *server_name, void *message, size_t message_len);
#endif
