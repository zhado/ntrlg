#include <cstdint>
#include <cstdlib>
#include <curses.h>
#include <string.h>

#include "logs.h"
struct size_n_index{
	int size;
	int index;
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
		int temp_size=b->size;
		int temp_index=b->index;
		b->size=a->size;
		b->index=a->index;
		a->size=temp_size;
		a->index=temp_index;
	}
}

void sort_sni_s(size_n_index* sni,int count){
	for(int i=0;i<count;i++){
		int min=sni[i].size,
		    min_index=i;
		for(int j=i;j<count;j++){
			if(sni[j].size<min){
				min=sni[j].size;
				min_index=j;
			}
			
		}
		swap_sni_s(&sni[i], &sni[min_index]);
	}

}
void match_names(int row, int col,t_log* log_p, char* search_string){
	int count=log_p->index;
	size_n_index* evaled_names_ar=(size_n_index*)malloc(sizeof(size_n_index)*count);
	for(int i=0;i<count;i++){
		evaled_names_ar[i].size=match_score(log_p->entries[i].name,search_string);
		evaled_names_ar[i].index=i;
	}
	sort_sni_s(evaled_names_ar, count);

	for(int i=0;i<log_p->index;i++){
		if(evaled_names_ar[i].size!=0)
			break;
		mvprintw(row-i,col,"%s %d",log_p->entries[evaled_names_ar[i].index].name,evaled_names_ar[i].size);
	}
	free(evaled_names_ar);
}

