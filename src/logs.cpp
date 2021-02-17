#include <stdio.h>
#include <cstdlib>
#include <string.h>

#include "logs.h"
#include "trlg_common.h"
#include "trlg_string.h"

int get_tag_id(t_log* log_p, strPart prt){
	char* name;
	char temp_tag_str[MAX_NAME_SIZE];
	if(prt.length==-1){
		name=prt.start;
	}else{
		memcpy(temp_tag_str, prt.start, prt.length);
		temp_tag_str[prt.length]=0;
		name=temp_tag_str;
		remove_commas_from_end(temp_tag_str);
	}

	for(int i=0;i<log_p->tg_count;i++){
		char* tag_str=log_p->tg_enrtries[i].tag;
		if(strcmp(tag_str, name)==0){
			return i;
		}
	}
	return -1;
}

char* get_str_from_id(t_log* log_p, int tag_id){
	return log_p->tg_enrtries[tag_id].tag;
}

void reconstruct_tags(t_log* log_p,log_entry* entry,char* str){
	for(int i=0;;i++){
		int cur_id=entry->tags[i];
		if(cur_id==0)
			break;
		if(i!=0)
			strncat(str,",",MAX_NAME_SIZE);
		char* name=get_str_from_id(log_p,cur_id);
		strncat(str,name,MAX_NAME_SIZE-1);
	}
}

int add_tag(t_log* log_p,strPart prt){
	char* name;
	char temp_tag_str[MAX_NAME_SIZE];
	if(prt.length==-1){
		name=prt.start;
	}else{
		memcpy(temp_tag_str, prt.start, prt.length);
		temp_tag_str[prt.length]=0;
		name=temp_tag_str;
	}

	int index=log_p->tg_count;
	if(index>=log_p->tg_alloced)
		return -1;

	//if(find_tag_id(log_p, name)!=-1)
		//return 0;

	strncpy(log_p->tg_enrtries[index].tag, name, MAX_NAME_SIZE);
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

void generate_entry_tags(t_log* log_p,log_entry* entry,char* sub_name){
	for(int i=0;;i++){
		strPart prt=get_nth_strpart(sub_name, ',', i);
		if(prt.length==0){
			break;
		}
		
		int new_tag_id=get_tag_id(log_p, prt);

		if(new_tag_id==-1)
			new_tag_id=add_tag(log_p,prt);
		entry->tags[i]=new_tag_id;
	}
}

void add_entry(t_log* log_p, char* name, char* sub_name,time_t start_time,time_t end_time){
	if(log_p->index!=0)
		end_last_entry(log_p);
	if(log_p->allocated < (log_p->index+1) ){
		log_p->allocated=log_p->allocated+REALLOC_INCREMENT;
		log_p->entries=(log_entry*)realloc(log_p->entries, sizeof(log_entry)*(log_p->allocated));
		for(int i=log_p->allocated-REALLOC_INCREMENT;i<log_p->allocated;i++){
			log_p->entries[i].name=(char*)calloc(sizeof(char)*MAX_NAME_SIZE,1);
		}
	}
	log_entry* entry=&log_p->entries[log_p->index];

	entry->end_time=end_time;

	entry->start_time=start_time;
	strcpy(entry->name, name);
	remove_spaces(sub_name);
	remove_commas_from_end(sub_name);
	for(int i=0;i<ENTRY_TAG_SIZE;i++){
		entry->tags[i]=0;
	}
	generate_entry_tags(log_p, entry, sub_name);

	log_p->index++;
}


int get_log_entry_index(t_log* a_log,log_entry* entry){
	int entry_size=sizeof(log_entry);
	return (entry-a_log->entries);
}

void remove_entry(t_log* log_p,log_entry* entry){
	int entry_index=get_log_entry_index(log_p, entry);
	for(int i=entry_index;i<log_p->index-1;i++){
		log_p->entries[i].end_time=log_p->entries[i+1].end_time;
		log_p->entries[i].start_time=log_p->entries[i+1].start_time;
		
		strcpy(log_p->entries[i].name,log_p->entries[i+1].name);
		//strcpy(log_p->entries[i].sub_name,log_p->entries[i+1].sub_name);
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
		if(entry->end_time>=curs_start && entry->end_time <=curs_end){
			if((entry->end_time-entry->start_time) > last_duration){
				last_duration=entry->end_time-entry->start_time;
				longest_entry=entry;
				match_type=3;
			}
		}else if(entry->end_time==0 && current_time >= curs_start && current_time <=curs_end){
			match_type=3;
			longest_entry=entry;
		}else if(entry->end_time==0 && current_time >= curs_end && entry->start_time <=curs_start){
			match_type=2;
			longest_entry=entry;
		}else if(entry->start_time >= curs_start && entry->start_time <=curs_end && last_duration==0){
			match_type=1;
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
