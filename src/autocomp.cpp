#include <cstdint>
#include <cstdlib>
#include <curses.h>
#include <string.h>

#include "logs.h"
struct size_n_index{
	int score;
	int index;
	char* offset;
	int size;
};

struct match_result{
	int match_count;
	char* requested_str;
	int size;
};

int match_score(char* st1, char* st2){
	int res=0,
	    res_t=0,
	    l1=strlen(st1),
	    l2=strlen(st2),
	    min=INT32_MAX;

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
	return min;
}

void swap_sni_s(size_n_index* a, size_n_index* b){
	if(a!=b){
		int temp_score=b->score;
		int temp_index=b->index;
		char* temp_offset=b->offset;
		int temp_size=b->size;

		b->score=a->score;
		b->index=a->index;
		b->offset=a->offset;
		b->size=a->size;
		a->score=temp_score;
		a->index=temp_index;
		a->offset=temp_offset;
		a->size=temp_size;
	}
}

void sort_sni_s(size_n_index* sni,int count){
	for(int i=0;i<count;i++){
		int min=sni[i].score,
		    min_index=i;
		for(int j=i;j<count;j++){
			if(sni[j].score<min){
				min=sni[j].score;
				min_index=j;
			}
			
		}
		swap_sni_s(&sni[i], &sni[min_index]);
	}

}

char* get_after_last_comma (char* str){
	int last_comma_pos=0;
	for(int i=0;i<strlen(str);i++){
		if(str[i]==','){
			last_comma_pos=i;
		}
	}
	return str+last_comma_pos;
}

match_result match_names(int row, int col,t_log* log_p, char* search_string, int choice){
	//extract last mdzime
	//extract all mdzimeebi into array
	int count=log_p->index;
	int comma_count=count;
	int rev=0;
	match_result res;

	for(int i=0;i<count;i++){
		log_entry* entry=&log_p->entries[i];
		for(int j=0;j<strlen(entry->sub_name);j++){
			if(entry->sub_name[j]==','){
				comma_count++;
			}
		}
	}
	size_n_index evaled_names_ar[comma_count];

	for(int i=0,j=0;i<count;i++,j++){
		char* entry_char=log_p->entries[i].sub_name;
		int len=strlen(entry_char);
		char* start_at=entry_char;
		char tempchar[len];
		memset(tempchar, 0, len);
		for(int k=0;k<len;k++){
			if(entry_char[k]==','){
				memset(tempchar, 0, len);
				memcpy(tempchar, start_at, entry_char-start_at+k);
				tempchar[entry_char-start_at+k]=0;
				evaled_names_ar[j].score=match_score(tempchar,search_string);
				evaled_names_ar[j].index=i;
				evaled_names_ar[j].offset=start_at;
				evaled_names_ar[j].size=entry_char-start_at+k;
				start_at=&entry_char[k]+1;
				j++;
			}
		}

		if(len!=0)
			memcpy(tempchar, start_at, entry_char+len-start_at-1);
		evaled_names_ar[j].score=match_score(tempchar,search_string);
		evaled_names_ar[j].index=i;
		evaled_names_ar[j].offset=start_at;
		evaled_names_ar[j].size=entry_char+len-start_at;
	}

	sort_sni_s(evaled_names_ar, count);

	for(int i=0;i<comma_count;i++){
		char tempchar[evaled_names_ar[i].size];
		memset(tempchar, 0, evaled_names_ar[i].size);
		if(evaled_names_ar[i].score!=0)
			break;
		if(choice == i)
		attron(COLOR_PAIR(2));
		memcpy(tempchar, evaled_names_ar[i].offset,evaled_names_ar[i].size);
		tempchar[evaled_names_ar[i].size]=0;
		mvprintw(row-i,col,"%s %d",tempchar,evaled_names_ar[i].score);
		attroff(COLOR_PAIR(2));
		res.match_count=i;
	}
	if(choice!=-1){
		res.requested_str= evaled_names_ar[choice].offset;
		res.size= evaled_names_ar[choice].size;
	}
	return res;
}
