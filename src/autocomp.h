#ifndef AUTOCOMP_H 
#define AUTOCOMP_H

#include "logs.h"
#include "trlg_common.h"
#include "trlg_string.h"

typedef struct {
	int tag_id;
	log_entry* root_entry;
}scoredTag;

typedef struct {
	int match_count;
	char* requested_str;
	int size;
}match_result;

void match_names(t_log* log_p, wchar_t* search_string_p, scoredTag* output, int* matched_count);
void draw_autocomp(int row, int col,t_log* log_p,scoredTag sni[AUTOCOM_WIN_MAX_SIZE], int choice,int matched_count);
bool entry_has_tag(log_entry* entry,int tag_id);

#endif
