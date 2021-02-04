#include <cstring>
#include <ncurses.h>

#include "logs.h"
#include "autocomp.h"
#include "draw.h"
#include "trlg_common.h"
#include "trlg_string.h"
#include "stats.h"


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

int get_tag_color_pair(char* str, statConfig* stat_conf){
	char temp_str[MAX_NAME_SIZE];
	// tracked tags loop.
	for(int i=0;i<stat_conf->count;i++){
		memcpy(temp_str, stat_conf->stat_colors[i].part.start, stat_conf->stat_colors[i].part.length);
		temp_str[stat_conf->stat_colors[i].part.length]=0;

		if(str[0]==0)
			return 0;
		char* nxt_comma;
		char* prv_comma=str;

		bool found=false;

		// extract entry-tags to test track-tags to.
		while(1){
			nxt_comma = next_comma(prv_comma);
			strPart str_part={prv_comma,(int)(nxt_comma-prv_comma)};

			if(nxt_comma==0){
				nxt_comma=prv_comma;
				str_part.length=str+strlen(str)-nxt_comma;
			}
			for(int j=0;j<str_part.length;j++){
				if(str_part.start[j]!=temp_str[j]){
					found=false;
					break;
				}else{
					found=true;
				}
			}
			if(found){
				return stat_conf->stat_colors[i].pair_id;
			}

			if(nxt_comma==prv_comma){
				break;
			}
			prv_comma=nxt_comma+1;
		}
	}
	return 0;
}
statColor get_statcolor(strPart strprt){
	statColor col={0,-1,-1,-1};
	int first_p=-1;
	int sec_p=-1;
	char* str=strprt.start;

	for(int i=0;i<strprt.length;i++){
		if(str[i]=='(')
			first_p=i;
		else if(str[i]==')')
			sec_p=i;
	}

	col.part.start=str;
	if(first_p!=-1)
		col.part.length=first_p;
	else
		col.part.length=strprt.length;

	if(first_p ==-1 || sec_p ==-1){
		return col;
	}

	sscanf(str+first_p+1,"%d %d",&col.bg,&col.fg);
	//printf("%d %d\n",col.bg,col.fg);
	return col;
}

statConfig generate_stat_colors(char* str){
	statConfig conf;
	conf.count=0;

	if(str[0]==0)
		return conf;
	char* nxt_comma;
	char* prv_comma=str;

	int count=0;
	while(1){
		nxt_comma = next_comma(prv_comma);
		strPart str_part={prv_comma,(int)(nxt_comma-prv_comma)};

		if(nxt_comma==0){
			nxt_comma=prv_comma;
			str_part.length=str+strlen(str)-nxt_comma;
		}

		conf.stat_colors[count]=get_statcolor(str_part);

		short fg=conf.stat_colors[count].fg;
		short bg=conf.stat_colors[count].bg;
		init_pair(count+10,fg ,bg);
		if(fg==-1 && bg==-1)
			conf.stat_colors[count].pair_id=0;
		else
			conf.stat_colors[count].pair_id=count+10;


		count++;
		if(nxt_comma==prv_comma){
			break;
		}
		prv_comma=nxt_comma+1;
	}
	conf.count=count;

	return conf;
}

time_t get_duration_in_range(t_log* a_log, char* str,time_t start_tm,time_t end_tm){
	time_t duration=0;
	if(strlen(str)==0)return duration;
	for(int i=a_log->index-1;i>=0;i--){
		log_entry cur_entry=a_log->entries[i];
		if(cur_entry.end_time==0)cur_entry.end_time=(unsigned long)time(0);
		if(cur_entry.end_time < start_tm)
			break;
		if(match_scores_by_comma(cur_entry.sub_name, str)==0){
			time_t temp_start_tm=tm_clamp(start_tm, cur_entry.start_time, cur_entry.end_time);
			time_t temp_end_tm=tm_clamp(end_tm, cur_entry.start_time, cur_entry.end_time);
			duration+=temp_end_tm-temp_start_tm;
		}
	}
	return duration;
}

void draw_durations(int row, int col,t_log* a_log, statConfig* stat_conf){
	int last_days= 7;
	time_t local_time=(unsigned long)time(NULL);
	time_t secs_in_day=24*60*60;
	char temp_str[MAX_NAME_SIZE];

	int start_row=row;
	for(int i=0;i<stat_conf->count;i++){
		memcpy(temp_str, stat_conf->stat_colors[i].part.start, stat_conf->stat_colors[i].part.length);
		temp_str[stat_conf->stat_colors[i].part.length]=0;

		int color=0;
		if(stat_conf!=0)
			color=get_tag_color_pair(temp_str, stat_conf);
		attron(COLOR_PAIR(color));
		print_str_n_times(row, col, " ", 82);
		move(row++,col);
		if(temp_str[0]!=0)
			printw("%s: ",temp_str);

		tm broken_down_time=get_tm(local_time);
		time_t last_midnight=local_time-broken_down_time.tm_hour*60*60-broken_down_time.tm_min*60-broken_down_time.tm_sec;
		
		for(int j=0;j<=last_days;j++){
				time_t start_time=last_midnight - secs_in_day*j;
				time_t end_time=last_midnight - secs_in_day*(j-1);
				if(row==start_row+1){
					attroff(COLOR_PAIR(color));
					mvprintw(start_row-2,col+12+9*j+1,"%02d",get_tm(start_time).tm_mday);
					attron(COLOR_PAIR(color));
				}
				move(row-1,col+12+9*j);
				printw("|");
				
				print_duration(get_duration_in_range(a_log, temp_str,start_time,end_time));

		}
		attroff(COLOR_PAIR(color));
	}
}

