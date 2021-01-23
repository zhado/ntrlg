#ifndef AUTOCOMP_H 
#define AUTOCOMP_H

#include "logs.h"

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

int match_score(char* st1, char* st2);
char* get_after_last_comma (char* str);
char* remove_spaces(char* str);
match_result match_names(int row, int col,t_log* log_p, char* search_string_p, int choice,bool remove_dups);

#endif
