#include <cstdint>
#include <cstdlib>
#include <curses.h>
#include <string.h>

#include "logs.h"
#include "trlg_common.h"
#include "trlg_string.h"
#include "draw.h"
#include "autocomp.h"

int match_score_prt(strPart ptr1, char* str2){
	int res=0,
	    l1=ptr1.length,
	    l2=strlen(str2),
	    min=INT32_MAX;

	if(l2>l1) {
		res=INT32_MAX;
		return res;
	}

	for(int i=0;i<=l1-l2;i++){
		for(int j=0;j<l2;j++){
			 res+= *(ptr1.start+j+i) ^ *(str2+j);
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

bool tag_has_str(char* str, char* search_str){
	int score;
	for(int i=0;;i++){
		strPart prt=get_nth_strpart(str, ',', i);
		if(prt.length==0){
			break;
		}

		if(match_score_prt(prt,search_str)==0)
			return true;
	}
	return false;
}

void remove_scored_tag(scoredTag* sT, int* tag_count,int index){
	for(int i=index;i<*tag_count-1;i++){
		sT[i].offset=sT[i+1].offset;
		sT[i].root_entry=sT[i+1].root_entry;
		sT[i].size=sT[i+1].size;
	}
	*tag_count=*tag_count-1;
}

bool comp_scored_tags(scoredTag sT_1, scoredTag sT_2){
	char temp_str_1[sT_1.size+1];
	memcpy(temp_str_1, sT_1.offset, sT_1.size);
	temp_str_1[sT_1.size]=0;

	char temp_str_2[sT_2.size+1];
	memcpy(temp_str_2, sT_2.offset, sT_2.size);
	temp_str_2[sT_2.size]=0;

	int comp_result=strcmp(temp_str_1, temp_str_2);

	if(comp_result==0)
		return true;
	else 
		return false;
}

void remove_dup_and_empty_scored_tags(scoredTag* sT, int* tag_count){
	if(*tag_count>0)
		if(sT[0].offset[0]==0){
			remove_scored_tag(sT, tag_count,0);
		}
	for(int i=0;i<*tag_count;i++){
		for(int j=i+1;j<*tag_count;j++){
			scoredTag sT_1=sT[i];
			scoredTag sT_2=sT[j];
			if(comp_scored_tags(sT[i], sT[j]) || sT_2.offset[0]==0){
				remove_scored_tag(sT, tag_count,j);
				j--;
			}
		}
	}
}

int generate_scored_tags(t_log* log_p, char * search_string, scoredTag* scored_tags){

	int j=0;
	for(int k=0;k<log_p->index;k++){
		int reverse_index=log_p->index-k-1;
		char* entry_subname=log_p->entries[reverse_index].sub_name;

		for(int i=0;;i++,j++){
			strPart prt=get_nth_strpart(entry_subname, ',', i);
			if(prt.length==0){
				break;
			}

			int score=match_score_prt(prt,search_string);
			if(score!=0){
				j--;
				continue;
			}

			scored_tags[j].offset=prt.start;
			scored_tags[j].size=prt.length;
			scored_tags[j].root_entry=&log_p->entries[reverse_index];
		}

	}


	return j;
}

void match_names(t_log* log_p, char* search_string, scoredTag* output, int* matched_count){
	//extract last mdzime
	char search_string_no_space[MAX_NAME_SIZE];
	int count=log_p->index;
	int tag_count=count;

	search_string=get_after_last_comma(search_string);
	strcpy(search_string_no_space, search_string);
	remove_spaces(search_string_no_space);

	for(int i=0;i<count;i++){
		log_entry* entry=&log_p->entries[i];
		for(int j=0;j<strlen(entry->sub_name);j++){
			if(entry->sub_name[j]==','){
				tag_count++;
			}
		}
	}

	scoredTag scored_tags[tag_count];

	tag_count=generate_scored_tags(log_p,search_string_no_space,scored_tags);
	remove_dup_and_empty_scored_tags(scored_tags,&tag_count);

	memcpy(output, scored_tags, sizeof(scoredTag)*AUTOCOM_WIN_MAX_SIZE);

	int i=0;
	for(;i<AUTOCOM_WIN_MAX_SIZE;i++){
		scoredTag cur_sT=scored_tags[i];
		//if( i > AUTOCOM_WIN_MAX_SIZE|| i>= tag_count){
		if( i > AUTOCOM_WIN_MAX_SIZE|| i>= tag_count){
			break;
		}
	}
	*matched_count=i;
}

void draw_autocomp(int row, int col,scoredTag sTs[AUTOCOM_WIN_MAX_SIZE], int choice,int matched_count){
	for(int i=0;i<matched_count;i++){
		scoredTag cur_sT=sTs[i];
		char tempchar[cur_sT.size+1];
		print_str_n_times(row-i, col-10, " ", 55);
		if(choice == i){
			attron(COLOR_PAIR(2));
			print_str_n_times(row-i, col-10, "- ", 55);
		}
		memcpy(tempchar, cur_sT.offset,cur_sT.size);
		tempchar[cur_sT.size]=0;
		//mvprintw(row-i,col,"| %s %d",tempchar,cur_sT.score);
		mvprintw(row-i,col,"| %s",tempchar);
		//mvprintw(row-i,col,"| %s",tempchar);
		attroff(COLOR_PAIR(2));
	}
}
