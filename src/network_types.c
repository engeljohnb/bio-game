#include "network_types.h"
#include "utils.h"

char *get_port(void)
{
	return TEMP_PORT;
}

void free_message(Message message)
{
	BG_FREE(message.data);
}
