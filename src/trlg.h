#ifndef TRLG_H 
#define TRLG_H

#include "logs.h"
#include "stats.h"

typedef struct {
	t_log logs;
	wchar_t* stat_input;
	statConfig stat_conf;
}app_state;

typedef struct {
	int port;
	int my_port;
	char* ip;
}server_conf;

#endif
