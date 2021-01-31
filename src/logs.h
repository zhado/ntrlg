#ifndef LOGS_H 
#define LOGS_H

#include <time.h>

struct log_entry {
	char* name;
	char* sub_name;
	time_t start_time;
	time_t end_time;
};

struct t_log {
	int index;
	int allocated;
	log_entry* entries;
};

bool crash_with_other_entry(t_log* a_log,log_entry* entry);

#endif
