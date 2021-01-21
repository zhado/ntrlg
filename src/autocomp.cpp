#include <cstdint>
#include <cstdlib>
#include <curses.h>
#include <string.h>

#include "logs.h"
#include "main.h"
#include "draw.h"

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

void remove_duplicate_and_empty_sni_s(size_n_index* sni, int* comma_count){
	for(int i=1;i<*comma_count;i++){
		size_n_index* cur_entry=&sni[i];
		size_n_index* prev_entry=&sni[i-1];
		char temp_str1[cur_entry->size];
		char temp_str2[prev_entry->size];
		memset(temp_str1, 0, cur_entry->size);
		memset(temp_str2, 0, prev_entry->size);
		memcpy(temp_str1, cur_entry->offset, cur_entry->size);
		memcpy(temp_str2, prev_entry->offset, prev_entry->size);
		temp_str1[cur_entry->size]=0;
		temp_str2[prev_entry->size]=0;
		int comp_result=strcmp(temp_str1, temp_str2);
		if((cur_entry->size==prev_entry->size && comp_result==0) || prev_entry->size==0 ){
			memcpy(prev_entry, cur_entry, sizeof(size_n_index)*(*comma_count-i));
			*comma_count=*comma_count-1;
			i--;
		}
	}
}

char* get_after_last_comma (char* str){
	int last_comma_pos=0;
	for(int i=0;i<strlen(str);i++){
		if(str[i]==','){
			last_comma_pos=i+1;
		}
	}
	return str+last_comma_pos;
}

char* remove_spaces(char* str){
	int len=strlen(str);
	for(int i=0;i<len;i++){
		if(str[i]==' '){
			memcpy(str+i, str+i+1,len-i);
			i--;
		}
	}
	return str;
}

match_result match_names(int row, int col,t_log* log_p, char* search_string_p, int choice,bool remove_dups){
	//extract last mdzime
	search_string_p=get_after_last_comma(search_string_p);
	char search_string[MAX_NAME_SIZE];
	strcpy(search_string, search_string_p);
	remove_spaces(search_string);
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
				evaled_names_ar[j].root_entry=&log_p->entries[i];
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
		evaled_names_ar[j].root_entry=&log_p->entries[i];
	}

	sort_sni_s(evaled_names_ar, comma_count);
	remove_duplicate_and_empty_sni_s(evaled_names_ar,&comma_count);

	for(int i=0;i<comma_count;i++){
		size_n_index cur_sni=evaled_names_ar[i];
		char tempchar[cur_sni.size];
		memset(tempchar, 0, cur_sni.size);
		if(cur_sni.score >= cur_sni.size+1 || i > AUTOCOM_WIN_MAX_SIZE)
			break;
		print_str_n_times(row-i, col-10, " ", 55);
		if(choice == i){
			attron(COLOR_PAIR(2));
			print_str_n_times(row-i, col-10, "- ", 55);
		}
		memcpy(tempchar, cur_sni.offset,cur_sni.size);
		tempchar[cur_sni.size]=0;
		mvprintw(row-i,col,"| %s %d",tempchar,cur_sni.score);
		//mvprintw(row-i,col,"| %s",tempchar);
		attroff(COLOR_PAIR(2));
		res.match_count=i;
	}
	if(choice!=-1){
		res.requested_str= evaled_names_ar[choice].offset;
		res.size= evaled_names_ar[choice].size;
	}
	return res;
}
