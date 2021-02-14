#ifndef LOGS_H 
#define LOGS_H

#include <time.h>
#include "trlg_common.h"

struct tgEntry{
	char tag[MAX_NAME_SIZE] = {0};
};

struct log_entry {
	char* name;
	char* sub_name;
	int tags[20]={0};
	time_t start_time;
	time_t end_time;
};

struct t_log {
	int index;
	int allocated;
	log_entry* entries;

	tgEntry* tg_enrtries;
	int tg_len;
	int tg_count;

};

bool crash_with_other_entry(t_log* a_log,log_entry* entry);
int get_log_entry_index(t_log* a_log,log_entry* entry);
int get_tag_id(t_log* log_p, char* name);
char* get_name_from_id(t_log* log_p, int tag_id);

#endif
