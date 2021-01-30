#include <cstring>
#include <ncurses.h>

#include "logs.h"
#include "autocomp.h"
#include "draw.h"
#include "trlg_common.h"
#include "trlg_string.h"

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
	if(strlen(str)==0)return duration;
	for(int i=0;i<a_log->index;i++){
		log_entry cur_entry=a_log->entries[i];
		if(cur_entry.end_time==0)cur_entry.end_time=(unsigned long)time(0);
		if(match_scores_by_comma(cur_entry.sub_name, str)==0){
			time_t temp_start_tm=tm_clamp(start_tm, cur_entry.start_time, cur_entry.end_time);
			time_t temp_end_tm=tm_clamp(end_tm, cur_entry.start_time, cur_entry.end_time);
			duration+=temp_end_tm-temp_start_tm;
		}
	}
	return duration;
}


void draw_durations(int row, int col,t_log* a_log, char* str){
	if(str==0)
		return;
	char my_str[MAX_NAME_SIZE];
	memset(&my_str,0,MAX_NAME_SIZE);
	strcpy(my_str, str);
	remove_spaces(my_str);

	int last_days= 7;
	time_t local_time=(unsigned long)time(NULL);
	time_t secs_in_day=24*60*60;

	char temp_str[MAX_NAME_SIZE];
	char* ch_start_p=my_str;

	int start_row=row;
	for(;;){
		memset(&temp_str,0,MAX_NAME_SIZE);

		char* n_comma=next_comma(ch_start_p);
		if(n_comma==0){
			memcpy(temp_str,ch_start_p,strlen(my_str));
		}else{
			memcpy(temp_str,ch_start_p,n_comma-ch_start_p);
		}

		move(row++,col);
		if(temp_str[0]!=0)
			printw("%s: ",temp_str);

		tm broken_down_time=get_tm(local_time);
		time_t last_midnight=local_time-broken_down_time.tm_hour*60*60-broken_down_time.tm_min*60-broken_down_time.tm_sec;
		
		for(int i=0;i<=last_days;i++){
				time_t start_time=last_midnight - secs_in_day*i;
				time_t end_time=last_midnight - secs_in_day*(i-1);
				if(row==start_row+1){
					mvprintw(start_row-2,col+12+9*i+1,"%02d",get_tm(start_time).tm_mday);
					//move(start_row-3,col+12+9*i+1);
					//print_normal_date_time(start_time);
					//move(start_row-4,col+12+9*i+1);
					//print_normal_date_time(end_time);
				}
				move(row-1,col+12+9*i);
				printw("│");
				
				print_duration(get_duration_in_range(a_log, temp_str,start_time,end_time));
		}

		if(!n_comma || n_comma==ch_start_p+strlen(ch_start_p))break;
		ch_start_p=n_comma+1;
	}
}

