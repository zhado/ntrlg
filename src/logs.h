#ifndef LOGS_H 
#define LOGS_H

#include <stdbool.h>
#include <time.h>
#include <wchar.h>
#include "trlg_common.h"
#include "trlg_string.h"

typedef struct{
	wchar_t tag[MAX_NAME_SIZE];
}tgEntry;

typedef struct{
	wchar_t* name;
	int tags[MAX_TAG_COUNT];
	time_t start_time;
	time_t end_time;
}log_entry;

typedef struct {
	int index;
	int allocated;
	log_entry* entries;

	tgEntry* tg_enrtries;
	int* tg_recents;
	int tg_alloced;
	int tg_count;

}t_log ;

bool crash_with_other_entry(t_log* a_log,log_entry* entry,log_entry** crashed_entry_next, log_entry** crashed_entry_prev);
int get_log_entry_index(t_log* a_log,log_entry* entry);
int get_tag_id(t_log* log_p,strPart prt);
wchar_t* get_str_from_id(t_log* log_p, int tag_id);
void reconstruct_tags(t_log* log_p,log_entry* entry,wchar_t* str);

#endif
