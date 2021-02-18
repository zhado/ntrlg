#ifndef STATS_H 
#define STATS_H

#include "trlg_string.h"
#include "trlg_common.h"
#include "logs.h"

struct statColor{
	int tag;
	int fg;
	int bg;
	int pair_id;
};

struct statConfig{
	statColor stat_colors[MAX_STAT_COUNT];
	int stat_selection;
	int count;
};

int get_tag_color_pair(int* tags, statConfig* stat_conf, int index);
int get_entry_tag_count(log_entry* entry);
#endif
