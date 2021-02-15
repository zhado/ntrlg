#ifndef AUTOCOMP_H 
#define AUTOCOMP_H

#include "logs.h"
#include "trlg_common.h"

struct scoredTag{
	int tag_id;
	log_entry* root_entry;
};

struct match_result{
	int match_count;
	char* requested_str;
	int size;
};

int match_score(char* st1, char* st2, bool exact_match);
char* get_after_last_comma (char* str);
char* remove_spaces(char* str);
void match_names(t_log* log_p, char* search_string_p, scoredTag* output, int* matched_count);
void draw_autocomp(int row, int col,t_log* log_p,scoredTag sni[AUTOCOM_WIN_MAX_SIZE], int choice,int matched_count);
bool entry_has_tag(log_entry* entry,int tag_id);

#endif
