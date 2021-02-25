#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "trlg_common.h"
#include "logs.h"

bool crash_with_other_entry(t_log* a_log,log_entry* entry){
	log_entry* next_entry=0;
	log_entry* prev_entry=0;
	time_t local_time=(unsigned long)time(NULL);

	int res_entry_index=get_log_entry_index(a_log, entry);

	if(res_entry_index!=a_log->index-1)
		next_entry=&a_log->entries[res_entry_index+1];
	if(res_entry_index!=0)
		prev_entry=&a_log->entries[res_entry_index-1];

	if(prev_entry ==0 && next_entry==0){
		if(entry->start_time >= entry->end_time){
			return true;
		}
	}else if(prev_entry ==0){
		if(entry->end_time >next_entry->start_time && next_entry!=0){
			return true;
		}else if(entry->start_time >= entry->end_time){
			return true;
		}
	}else if(next_entry==0){
		if(entry->start_time < prev_entry->end_time){
			return true;
		}else if(entry->end_time!=0){
			if(entry->start_time >= entry->end_time){
				return true;
			}
		}else{
			if(entry->start_time >= local_time){
				return true;
			}
		}

	}else{
		if(entry->start_time<prev_entry->end_time){
			return true;
		}else if(entry->end_time >next_entry->start_time && next_entry!=0){
			return true;
		}else if(entry->start_time >= entry->end_time){
			return true;
		}
	}
	if(entry->end_time>local_time || entry->start_time > local_time)
		return	true;

	return false;
}

void resize_logic(time_t* cursor_pos_tm, int* cell_minutes,  log_entry* entry_to_resize,t_log* log_p, int chr, window_state* win_state){

	time_t initial_cursor_pos_tm=*cursor_pos_tm;
	time_t local_time=(unsigned long)time(0);
	time_t initial_start_time=entry_to_resize->start_time;
	time_t initial_end_time=entry_to_resize->end_time;

	switch(chr){
		case 259:{
			//uparrow
			*cursor_pos_tm-=*cell_minutes*60;
		}break;
		case 10:{
			*win_state=view;
		}break;
		case 'q':{
			*win_state=view;
		}break;
		case 258:{
			//downarrow
			*cursor_pos_tm+=*cell_minutes*60;
		}break;  
		case 339:{
			//pgup
			*cursor_pos_tm-=*cell_minutes*60*4;
		}break; 
		case 338:{
			//pgdown
			*cursor_pos_tm+=*cell_minutes*60*4;
		}case 'z':{
			if(*cell_minutes!=5){
				*cell_minutes=*cell_minutes-5;
			}
		}break;
		case 'x':{
			*cell_minutes=*cell_minutes+5;
		}break;
	}

	if(*win_state==entry_end_resize){
		entry_to_resize->end_time=*cursor_pos_tm;
	}else if(*win_state==entry_start_resize) {
		entry_to_resize->start_time=*cursor_pos_tm;
	}else if(*win_state==entry_body_resize) {
		if(entry_to_resize->end_time==0)
			entry_to_resize->end_time=local_time;
		entry_to_resize->start_time+=*cursor_pos_tm-initial_cursor_pos_tm;
		entry_to_resize->end_time+=*cursor_pos_tm-initial_cursor_pos_tm;
	}

	if(crash_with_other_entry(log_p, entry_to_resize)){
		entry_to_resize->start_time=initial_start_time;
		entry_to_resize->end_time=initial_end_time;
		*cursor_pos_tm=initial_cursor_pos_tm;
	}

}
