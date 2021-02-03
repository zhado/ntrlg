#ifndef STATS_H 
#define STATS_H

#include "trlg_string.h"

struct statColor{
	strPart part;
	int fg;
	int bg;
	int pair_id;
};

struct statConfig{
	statColor stat_colors[50];
	int count;
};

int get_tag_color_pair(char* str, statConfig* stat_conf);
#endif
