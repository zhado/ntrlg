#include <cstdint>
#include <cstdlib>
#include <curses.h>
#include <string.h>

#include "logs.h"
struct size_n_index{
	int score;
	int index;
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

	for(int i=0;i<l1-l2;i++){
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
		int temp_size=b->score;
		int temp_index=b->index;
		b->score=a->score;
		b->index=a->index;
		a->score=temp_size;
		a->index=temp_index;
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
	int rev=0;
	match_result res;
	size_n_index evaled_names_ar[count];
	for(int i=0;i<count;i++){
		rev=count-i-1;
		evaled_names_ar[i].score=match_score(log_p->entries[rev].sub_name,search_string);
		evaled_names_ar[i].index=rev;
	}
	sort_sni_s(evaled_names_ar, count);

	for(int i=0;i<log_p->index;i++){
		if(evaled_names_ar[i].score!=0)
			break;
		if(choice == i)
			attron(COLOR_PAIR(2));
		mvprintw(row-i,col,"%s %d",log_p->entries[evaled_names_ar[i].index].sub_name,evaled_names_ar[i].score);
		attroff(COLOR_PAIR(2));
		res.match_count=i;
	}
	if(choice!=-1){
		res.requested_str= log_p->entries[evaled_names_ar[choice].index].sub_name;
	}
	return res;
}

