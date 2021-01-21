#include <cstring>
#include <ncurses.h>

#include "logs.h"
#include "autocomp.h"
#include "draw.h"

time_t tm_clamp(time_t in, time_t min, time_t max){
	if(in > min && in < max){
		return in;
	}else if(in >= max){
		return max;
	}else if(in <= min){
		return min;
	}
	return 0;
}

time_t get_duration_in_range(t_log* a_log, char* str,time_t start_tm,time_t end_tm){
	time_t duration=0;
	for(int i=0;i<a_log->index;i++){
		log_entry cur_entry=a_log->entries[i];
		if(cur_entry.end_time==0)cur_entry.end_time=(unsigned long)time(0);
		if(match_score(cur_entry.sub_name, str)==0){
			time_t temp_start_tm=tm_clamp(start_tm, cur_entry.start_time, cur_entry.end_time);
			time_t temp_end_tm=tm_clamp(end_tm, cur_entry.start_time, cur_entry.end_time);
			duration+=temp_end_tm-temp_start_tm;
		}
	}
	return duration;
}

char* next_comma(char* str){
	for(int i=0;i<strlen(str);i++){
		if(str[i]==','){
			return str+i;
		}
	}
	return 0;
}

void draw_durations(int row, int col,t_log* a_log, char* str,time_t start_tm,time_t end_tm){
	remove_spaces(str);

	char temp_str[strlen(str)];
	char* ch_start_p=str;

	for(;;){
		memset(temp_str,0,strlen(str));

		char* n_comma=next_comma(ch_start_p);
		if(n_comma==0){
			memcpy(temp_str,ch_start_p,strlen(str));
		}else{
			memcpy(temp_str,ch_start_p,n_comma-ch_start_p);
		}

		move(row++,col);
		printw("%s: ",temp_str);
		print_duration(get_duration_in_range(a_log, temp_str, start_tm, end_tm));

		if(!n_comma)break;
		ch_start_p=n_comma+1;
	}
	
	//move(row,col);
}
