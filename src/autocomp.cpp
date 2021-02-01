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
	    res_t=0,
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

	int last_days= 7;
	time_t local_time=(unsigned long)time(NULL);
	time_t secs_in_day=24*60*60;

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

void remove_sni(size_n_index* sni, int* tag_count,int index){
	for(int i=index;i<*tag_count-1;i++){
		sni[i].index=sni[i+1].index;
		sni[i].offset=sni[i+1].offset;
		sni[i].root_entry=sni[i+1].root_entry;
		sni[i].score=sni[i+1].score;
		sni[i].size=sni[i+1].size;
	}
	*tag_count=*tag_count-1;
}

void null_sni(size_n_index* sni,int index){
	sni[index].index=0;
	sni[index].offset=0;
	sni[index].root_entry=0;
	sni[index].score=INT32_MAX;
	sni[index].size=0;
}

bool cmp_sni(size_n_index sni1, size_n_index sni2){
	char temp_str1[sni1.size+1];
	memset(temp_str1, 0, sni1.size+1);
	memcpy(temp_str1, sni1.offset, sni1.size);

	char temp_str2[sni2.size+1];
	memset(temp_str2, 0, sni2.size+1);
	memcpy(temp_str2, sni2.offset, sni2.size);

	int comp_result=strcmp(temp_str1, temp_str2);

	if(comp_result==0)
		return true;
	else 
		return false;
}

void remove_duplicate_and_empty_sni(size_n_index* sni, int* tag_count){
	if(*tag_count>0)
		if(sni[0].offset[0]==0){
			remove_sni(sni, tag_count,0);
		}
	for(int i=0;i<*tag_count;i++){
		for(int j=i+1;j<*tag_count-1;j++){
			size_n_index sni1=sni[i];
			size_n_index sni2=sni[j];
			if(cmp_sni(sni[i], sni[j])){
				remove_sni(sni, tag_count,j);
				j--;
			}
		}
	}
}

int generate_sni_s(t_log* log_p, int entry_count,int tag_count,char * search_string, size_n_index* evaled_names_ar){

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
				evaled_names_ar[j].score=match_score(tempchar,search_string,false);
				evaled_names_ar[j].index=reverse_index;
				evaled_names_ar[j].offset=start_at;
				evaled_names_ar[j].size=entry_char-start_at+k;
				evaled_names_ar[j].root_entry=&log_p->entries[reverse_index];
				start_at=&entry_char[k]+1;
				j++;
			}
		}

		if(len!=0){
			memcpy(tempchar, start_at, entry_char+len-start_at);
			tempchar[entry_char-start_at+len]=0;
		}

		evaled_names_ar[j].score=match_score(tempchar,search_string,false);
		evaled_names_ar[j].index=reverse_index;
		evaled_names_ar[j].offset=start_at;
		evaled_names_ar[j].size=entry_char+len-start_at;
		evaled_names_ar[j].root_entry=&log_p->entries[reverse_index];
	}

	return 0;
}

void match_names(t_log* log_p, char* search_string, bool remove_dups, size_n_index* output, int* matched_count){
	//extract last mdzime
	char search_string_no_space[MAX_NAME_SIZE];
	int count=log_p->index;
	int tag_count=count;
	int rev=0;
	match_result res;

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

	size_n_index evaled_names_ar[tag_count];


	generate_sni_s(log_p,count,tag_count,search_string_no_space,evaled_names_ar);
	sort_sni_s(evaled_names_ar, tag_count);
	remove_duplicate_and_empty_sni(evaled_names_ar,&tag_count);

	memcpy(output, evaled_names_ar, sizeof(size_n_index)*AUTOCOM_WIN_MAX_SIZE);

	int i=0;
	for(;i<AUTOCOM_WIN_MAX_SIZE;i++){
		size_n_index cur_sni=evaled_names_ar[i];
		//if( i > AUTOCOM_WIN_MAX_SIZE|| i>= tag_count){
		if( cur_sni.score!=0|| i > AUTOCOM_WIN_MAX_SIZE|| i>= tag_count){
			break;
		}
	}
	*matched_count=i;
}

void draw_sni(int row, int col,size_n_index sni[AUTOCOM_WIN_MAX_SIZE], int choice,int matched_count){
	for(int i=0;i<matched_count;i++){
		size_n_index cur_sni=sni[i];
		char tempchar[cur_sni.size+1];
		print_str_n_times(row-i, col-10, " ", 55);
		if(choice == i){
			attron(COLOR_PAIR(2));
			print_str_n_times(row-i, col-10, "- ", 55);
		}
		memcpy(tempchar, cur_sni.offset,cur_sni.size);
		tempchar[cur_sni.size+1]=0;
		//mvprintw(row-i,col,"| %s %d",tempchar,cur_sni.score);
		mvprintw(row-i,col,"| %s",tempchar);
		//mvprintw(row-i,col,"| %s",tempchar);
		attroff(COLOR_PAIR(2));
	}
}
