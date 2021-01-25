#ifndef AUTOCOMP_H 
#define AUTOCOMP_H

#include "logs.h"
#include "trlg_common.h"

struct size_n_index{
	int score;
	int index;
	char* offset;
	int size;
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
void match_names(t_log* log_p, char* search_string_p, bool remove_dups, size_n_index* output, int* matched_count);
void draw_sni(int row, int col,size_n_index sni[AUTOCOM_WIN_MAX_SIZE], int choice,int matched_count);
int match_scores_by_comma(char* str, char* search_strin);

#endif
