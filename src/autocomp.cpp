#include <cstdint>
#include <cstdlib>
#include <curses.h>
#include <string.h>

#include "logs.h"
#include "trlg_common.h"
#include "trlg_string.h"
#include "draw.h"
#include "autocomp.h"

int match_score(char* st1, char* st2,bool exact_match){
	int res=0,
	    l1=strlen(st1),
	    l2=strlen(st2),
	    min=INT32_MAX;
	if(exact_match){
		if(strcmp(st1, st2)==0){
			return 0;
		}else{
			return INT32_MAX;
		}
	}

	if(l2>l1) {
		res=INT32_MAX;
		return res;
	}

	for(int i=0;i<=l1-l2;i++){
		for(int j=0;j<l2;j++){
			 res+= *(st1+j+i) ^ *(st2+j);
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

int match_scores_by_comma(char* str, char* search_strin){
	int score=0,min=INT32_MAX;
	char my_str[MAX_NAME_SIZE];
	memset(&my_str,0,MAX_NAME_SIZE);
	strcpy(my_str, str);
	remove_spaces(my_str);

	char temp_str[MAX_NAME_SIZE];
	char* ch_start_p=my_str;

	for(;;){
		memset(&temp_str,0,MAX_NAME_SIZE);

		char* n_comma=next_comma(ch_start_p);
		if(n_comma==0){
			memcpy(temp_str,ch_start_p,strlen(my_str));
		}else{
			memcpy(temp_str,ch_start_p,n_comma-ch_start_p);
		}

		score=match_score(temp_str,search_strin,true);
		if(score<min){
			min=score;
		}
		if(!n_comma || n_comma==ch_start_p+strlen(ch_start_p))break;
		ch_start_p=n_comma+1;
	}
	return min;
}

void swap_scored_tags(scoredTag* a, scoredTag* b){
	if(a!=b){
		int temp_score=b->score;
		int temp_index=b->index;
		char* temp_offset=b->offset;
		int temp_size=b->size;
		log_entry* temp_entry=a->root_entry;

		b->score=a->score;
		b->index=a->index;
		b->offset=a->offset;
		b->size=a->size;
		b->root_entry=a->root_entry;

		a->score=temp_score;
		a->index=temp_index;
		a->offset=temp_offset;
		a->size=temp_size;
		a->root_entry=temp_entry;
	}
}

void sort_scored_tags(scoredTag* sT,int count){
	for(int i=0;i<count;i++){
		int min=sT[i].score,
		    min_index=i;
		for(int j=i;j<count;j++){
			if(sT[j].score<min){
				min=sT[j].score;
				min_index=j;
			}
			
		}
		swap_scored_tags(&sT[i], &sT[min_index]);
	}

}

void remove_scored_tag(scoredTag* sT, int* tag_count,int index){
	for(int i=index;i<*tag_count-1;i++){
		sT[i].index=sT[i+1].index;
		sT[i].offset=sT[i+1].offset;
		sT[i].root_entry=sT[i+1].root_entry;
		sT[i].score=sT[i+1].score;
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
		for(int j=i+1;j<*tag_count-1;j++){
			scoredTag sT_1=sT[i];
			scoredTag sT_2=sT[j];
			if(comp_scored_tags(sT[i], sT[j]) || sT_2.offset[0]==0){
				remove_scored_tag(sT, tag_count,j);
				j--;
			}
		}
	}
}

int generate_scored_tags(t_log* log_p, int entry_count,int tag_count,char * search_string, scoredTag* scored_tags){

	for(int i=0,j=0;i<entry_count;i++,j++){
		int reverse_index=entry_count-i-1;
		char* entry_char=log_p->entries[reverse_index].sub_name;
		int len=strlen(entry_char);
		char* start_at=entry_char;
		char tempchar[len];
		memset(tempchar, 0, len);
		for(int k=0;k<len;k++){
			if(entry_char[k]==','){
				memset(tempchar, 0, len);
				memcpy(tempchar, start_at, entry_char-start_at+k);
				tempchar[entry_char-start_at+k]=0;
				int score=match_score(tempchar,search_string,false);
				scored_tags[j].score=score;
				scored_tags[j].index=reverse_index;
				scored_tags[j].offset=start_at;
				scored_tags[j].size=entry_char-start_at+k;
				scored_tags[j].root_entry=&log_p->entries[reverse_index];
				start_at=&entry_char[k]+1;
				j++;
			}
		}

		if(len!=0){
			memcpy(tempchar, start_at, entry_char+len-start_at);
			tempchar[entry_char-start_at+len]=0;
		}

		int score=match_score(tempchar,search_string,false);
		scored_tags[j].score=score;
		scored_tags[j].index=reverse_index;
		scored_tags[j].offset=start_at;
		scored_tags[j].size=entry_char+len-start_at;
		scored_tags[j].root_entry=&log_p->entries[reverse_index];
	}

	return 0;
}

void match_names(t_log* log_p, char* search_string, bool remove_dups, scoredTag* output, int* matched_count){
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


	generate_scored_tags(log_p,count,tag_count,search_string_no_space,scored_tags);
	sort_scored_tags(scored_tags, tag_count);
	remove_dup_and_empty_scored_tags(scored_tags,&tag_count);

	memcpy(output, scored_tags, sizeof(scoredTag)*AUTOCOM_WIN_MAX_SIZE);

	int i=0;
	for(;i<AUTOCOM_WIN_MAX_SIZE;i++){
		scoredTag cur_sT=scored_tags[i];
		//if( i > AUTOCOM_WIN_MAX_SIZE|| i>= tag_count){
		if( cur_sT.score!=0|| i > AUTOCOM_WIN_MAX_SIZE|| i>= tag_count){
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
