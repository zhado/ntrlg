#ifndef STATS_H 
#define STATS_H

#include "trlg_string.h"
#include "trlg_common.h"
#include "logs.h"

typedef struct {
	int tag;
	int fg;
	int bg;
	int pair_id;
}statColor;

typedef struct {
	statColor stat_colors[MAX_STAT_COUNT];
	int stat_selection;
	int count;
}statConfig;

int get_tag_color_pair(int* tags, statConfig* stat_conf, int index);
int get_entry_tag_count(log_entry* entry);
#endif
