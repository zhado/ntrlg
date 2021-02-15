#include <cstdint>
#include <cstdlib>
#include <curses.h>
#include <string.h>

#include "logs.h"
#include "trlg_common.h"
#include "trlg_string.h"
#include "draw.h"
#include "autocomp.h"

int match_score_prt(char* str1, char* str2){
	int res=0,
	    l1=strlen(str1),
	    l2=strlen(str2),
	    min=INT32_MAX;

	if(l2>l1) {
		res=INT32_MAX;
		return res;
	}

	for(int i=0;i<=l1-l2;i++){
		for(int j=0;j<l2;j++){
			 res+= *(str1+j+i) ^ *(str2+j);
		}
		if(res<min){
			min=res;
		}
		res=0;
	}

	if(min<0)
		return INT32_MAX;
	return min;
}

bool entry_has_tag(log_entry* entry,int tag_id){
	for(int i=0;;i++){
		if(entry->tags[i]==0)
			break;
		if(entry->tags[i]==tag_id)
			return true;
	}
	return false;
}

int generate_scored_tags(t_log* log_p, char * search_string, scoredTag* scored_tags){
	int j=0;
	for(int i=log_p->tg_count-1;i>=0;i--,j++){
			int score=match_score_prt(log_p->tg_enrtries[i].tag, search_string);
			if(score!=0){
				j--;
				continue;
			}

			scored_tags[j].root_entry=&log_p->entries[i];
			scored_tags[j].tag_id=i;
	}
	return j;
}

void match_names(t_log* log_p, char* search_string, scoredTag* output, int* matched_count){
	//extract last mdzime
	char search_string_no_space[MAX_NAME_SIZE];

	search_string=get_after_last_comma(search_string);
	strcpy(search_string_no_space, search_string);
	remove_spaces(search_string_no_space);

	int tag_count=log_p->tg_count;

	scoredTag scored_tags[tag_count];

	tag_count=generate_scored_tags(log_p,search_string_no_space,scored_tags);
	memcpy(output, scored_tags, sizeof(scoredTag)*AUTOCOM_WIN_MAX_SIZE);

	int i=0;
	for(;i<AUTOCOM_WIN_MAX_SIZE;i++){
		scoredTag cur_sT=scored_tags[i];
		if( i > AUTOCOM_WIN_MAX_SIZE|| i>= tag_count){
			break;
		}
	}
	*matched_count=i;
}

void draw_autocomp(int row, int col,t_log* log_p,scoredTag sTs[AUTOCOM_WIN_MAX_SIZE], int choice,int matched_count){
	for(int i=0;i<matched_count;i++){
		scoredTag cur_sT=sTs[i];
		print_str_n_times(row-i, col-10, " ", 55);
		if(choice == i){
			attron(COLOR_PAIR(2));
			print_str_n_times(row-i, col-10, "- ", 55);
		}
		//mvprintw(row-i,col,"| %s %d",tempchar,cur_sT.score);
		mvprintw(row-i,col,"| %s",log_p->tg_enrtries[cur_sT.tag_id].tag);
		//mvprintw(row-i,col,"| %s",tempchar);
		attroff(COLOR_PAIR(2));
	}
}
