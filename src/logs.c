#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logs.h"
#include "trlg_common.h"
#include "trlg_string.h"

int get_tag_id(t_log* log_p, strPart prt){
	wchar_t* name;
	wchar_t temp_tag_str[MAX_NAME_SIZE];
	if(prt.length==-1){
		name=prt.start;
	}else{
		wmemcpy(temp_tag_str, prt.start, prt.length);
		temp_tag_str[prt.length]=0;
		name=temp_tag_str;
		remove_commas_from_end(temp_tag_str);
	}

	for(int i=0;i<log_p->tg_count;i++){
		wchar_t* tag_str=log_p->tg_enrtries[i].tag;
		if(wcscmp(tag_str, name)==0){
			return i;
		}
	}
	return -1;
}

wchar_t* get_str_from_id(t_log* log_p, int tag_id){
	return log_p->tg_enrtries[tag_id].tag;
}

void reconstruct_tags(t_log* log_p,log_entry* entry,wchar_t* str){
	for(int i=0;;i++){
		int cur_id=entry->tags[i];
		if(cur_id==0)
			break;
		if(i!=0)
			wcsncat(str,L",",MAX_NAME_SIZE);
		wchar_t* name=get_str_from_id(log_p,cur_id);
		wcsncat(str,name,MAX_NAME_SIZE-1);
	}
}

int add_tag(t_log* log_p,strPart prt){
	wchar_t* name;
	wchar_t temp_tag_str[MAX_NAME_SIZE];
	if(prt.length==-1){
		name=prt.start;
	}else{
		wmemcpy(temp_tag_str, prt.start, prt.length);
		temp_tag_str[prt.length]=0;
		name=temp_tag_str;
	}

	int index=log_p->tg_count;
	if(index>=log_p->tg_alloced)
		return -1;

	//if(find_tag_id(log_p, name)!=-1)
		//return 0;

	wcsncpy(log_p->tg_enrtries[index].tag, name, MAX_NAME_SIZE);
	log_p->tg_count++;
	return index;
}

void end_last_entry(t_log* log_p){
	log_entry* entry=&log_p->entries[log_p->index-1];
	if(entry->start_time==0){
		fprintf(stderr, "entry not started.");
	}else if(entry->end_time==0){
		entry->end_time=(unsigned long)time(0);
	}
}

void promote_tag(t_log* log_p,int tg_id){
	// moves repeated tags to the top of the array
	if(tg_id<log_p->tg_count-1){
		int temp_id=0;
		for(int i=0;i<log_p->tg_count;i++){
			if(log_p->tg_recents[i]==tg_id)
				temp_id=i;
		}

		for(int i=temp_id;i<log_p->tg_count-1;i++){
			log_p->tg_recents[i]=log_p->tg_recents[i+1];
		}
		log_p->tg_recents[log_p->tg_count-1]=tg_id;
	}
}

void generate_entry_tags(t_log* log_p,log_entry* entry,wchar_t* sub_name){
	for(int i=0;i<MAX_TAG_COUNT;i++){
		entry->tags[i]=0;
	}

	remove_spaces(sub_name);
	remove_commas_from_end(sub_name);
	for(int i=0;;i++){
		//parse tags
		strPart prt=get_nth_strpart(sub_name, ',', i);
		if(prt.length==0){
			break;
		}
		
		int new_tag_id=get_tag_id(log_p, prt);

		// if its a new tag, add it
		if(new_tag_id==-1){
			new_tag_id=add_tag(log_p,prt);
			log_p->tg_recents[new_tag_id]=new_tag_id;
		}else{
			promote_tag(log_p, new_tag_id);
		}
		entry->tags[i]=new_tag_id;
	}
}

void copy_entry(log_entry* entr1,log_entry* entr2){
	entr1->start_time=entr2->start_time;
	entr1->end_time=entr2->end_time;
	int i=0;
	for(;entr2->tags[i]!=0;i++){
		entr1->tags[i]=entr2->tags[i];
	}
	entr1->tags[i]=0;
	wcscpy(entr1->name, entr2->name);
}

void add_entry(t_log* log_p, wchar_t* name, wchar_t* sub_name,time_t start_time,time_t end_time, bool is_insert){
	if(sub_name[0]==0 && name[0]==0){
		return;
	}
	if(log_p->index!=0 && !is_insert) 
		end_last_entry(log_p);
	if(log_p->allocated < (log_p->index+1) ){
		log_p->allocated=log_p->allocated+REALLOC_INCREMENT;
		log_p->entries=(log_entry*)realloc(log_p->entries, sizeof(log_entry)*(log_p->allocated));
		for(int i=log_p->allocated-REALLOC_INCREMENT;i<log_p->allocated;i++){
			log_p->entries[i].name=(wchar_t*)calloc(sizeof(wchar_t)*MAX_NAME_SIZE,1);
		}
	}
	log_entry* entry=&log_p->entries[log_p->index];

	// if we are inserting we need to shift entries forward
	if(is_insert){
		for(int i=log_p->index;i>=0;i--){
			if(log_p->entries[i-1].end_time<start_time&& log_p->entries[i-1].end_time!=0 ){
				entry=&log_p->entries[i];
				break;
			}
			copy_entry(&log_p->entries[i], &log_p->entries[i-1]);
		}
	}

	entry->end_time=end_time;

	entry->start_time=start_time;
	wcscpy(entry->name, name);
	for(int i=0;i<ENTRY_TAG_SIZE;i++){
		entry->tags[i]=0;
	}
	generate_entry_tags(log_p, entry, sub_name);
	
	// if name is empty, use first tag for name
	if(name[0]==0){
		wchar_t* tag=get_str_from_id(log_p, entry->tags[0]);
		wcscpy(entry->name, tag);
	}

	log_p->index++;
}


int get_log_entry_index(t_log* a_log,log_entry* entry){
	return (entry-a_log->entries);
}

void remove_entry(t_log* log_p,log_entry* entry){
	int entry_index=get_log_entry_index(log_p, entry);
	for(int i=entry_index;i<log_p->index;i++){
		log_p->entries[i].end_time=log_p->entries[i+1].end_time;
		log_p->entries[i].start_time=log_p->entries[i+1].start_time;
		
		wcscpy(log_p->entries[i].name,log_p->entries[i+1].name);
		for(int j=0;j<MAX_TAG_COUNT;j++){
			log_p->entries[i].tags[j]=log_p->entries[i+1].tags[j];
			if(log_p->entries[i].tags[j]==0){
				break;
			}
		}
	}
	log_p->index=log_p->index-1;
}

log_entry* entry_under_cursor_fun(t_log* log_p,int cell_minutes,time_t cursor_pos_tm, int* match_p){
	log_entry* longest_entry=0;

	time_t current_time=(unsigned long)time(0);
	time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));

	time_t curs_start=quantized_cursor_pos_tm;
	time_t curs_end=curs_start+cell_minutes*60;
	time_t last_duration=0;

	//0 nomatch, 1 start match, 2 body match, 3 end match
	int match_type=0;
	for(int i=log_p->index-1;i>=0;i--){
		log_entry* entry=&log_p->entries[i];
		if(entry->end_time==0 && current_time >= curs_start && current_time <=curs_end){
			match_type=3;
			longest_entry=entry;
			break;
		}else if(entry->start_time >= curs_start && entry->start_time <=curs_end && last_duration==0){
			match_type=1;
			longest_entry=entry;
		}else if(entry->end_time>=curs_start && entry->end_time <=curs_end){
			if((entry->end_time-entry->start_time) > last_duration){
				last_duration=entry->end_time-entry->start_time;
				longest_entry=entry;
				match_type=3;
			}
		}else if(entry->end_time==0 && current_time >= curs_end && entry->start_time <=curs_start){
			match_type=2;
			longest_entry=entry;
		}else if(entry->start_time<=curs_start && entry->end_time >= curs_end){
			match_type=2;
			longest_entry=entry;
		}
	}
	if(match_p!=0)
		*match_p=match_type;
	return longest_entry;
}
