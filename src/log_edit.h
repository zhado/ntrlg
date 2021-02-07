#ifndef LOG_EDIT_H 
#define LOG_EDIT_H

#include "autocomp.h"
#include "logs.h"
#include "trlg_common.h"

struct log_edit_buffer{
	char name[MAX_NAME_SIZE];
	char sub_name[MAX_NAME_SIZE];
	t_log* a_log;
	int tag_autocomp_selection;
	match_result result;
	bool only_tag_str;
	scoredTag sni[AUTOCOM_WIN_MAX_SIZE];
	int matched_count;
	int cursor_row;
	int cursor_col;
	int local_curs_pos;
};
#endif
