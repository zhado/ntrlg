#include <cstring>
#include <ncurses.h>

#include "logs.h"
#include "autocomp.h"
#include "draw.h"
#include "trlg_common.h"
#include "trlg_string.h"
#include "logs.h"
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

int get_tag_color_pair(int* tags, statConfig* stat_conf){
	if(tags[0]==0)
		return 0;

	for(int i=0;;i++){
		if(tags[i]==0)
			break;
		int cur_tag=tags[i];
		for(int j=0;j<stat_conf->count;j++){
			if(stat_conf->stat_colors[j].tag==cur_tag)
				return stat_conf->stat_colors[j].pair_id;
		}
	}
	return 0;
}

statColor get_statcolor(t_log* log_p,strPart strprt){
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


	char* start=str;
	int len=0;
	if(first_p!=-1)
		len=first_p;
	else
		len=strprt.length;
	
	//TODO: add_tag ma jobia kopirebis gareshe qnas
	char temp_tag_str[MAX_NAME_SIZE];
	memcpy(temp_tag_str,str,len);
	temp_tag_str[len]=0;
	col.tag=get_tag_id(log_p, temp_tag_str);

	if(first_p ==-1 || sec_p ==-1){
		return col;
	}

	sscanf(str+first_p+1,"%d %d",&col.bg,&col.fg);
	//printf("%d %d\n",col.bg,col.fg);
	return col;
}

statConfig generate_stat_colors(t_log* log_p,char* str){
	statConfig conf;
	conf.count=0;

	if(str[0]==0)
		return conf;
	char* nxt_comma;
	char* prv_comma=str;

	int count=0;
	for(int i=0;;i++){
		strPart prt=get_nth_strpart(str, ',', i);
		if(prt.length==0){
			break;
		}
		conf.stat_colors[count]=get_statcolor(log_p,prt);

		short fg=conf.stat_colors[count].fg;
		short bg=conf.stat_colors[count].bg;
		init_pair(count+10,fg ,bg);
		if(fg==-1 && bg==-1)
			conf.stat_colors[count].pair_id=0;
		else
			conf.stat_colors[count].pair_id=count+10;
		count++;
	}

	conf.count=count;

	return conf;
}

time_t get_duration_in_range(t_log* a_log, int tag_id,time_t start_tm,time_t end_tm){
	time_t duration=0;
	for(int i=a_log->index-1;i>=0;i--){
		log_entry cur_entry=a_log->entries[i];
		if(cur_entry.end_time==0)cur_entry.end_time=(unsigned long)time(0);
		if(cur_entry.end_time < start_tm)
			break;
		if(entry_has_tag(&cur_entry, tag_id)){
			time_t temp_start_tm=tm_clamp(start_tm, cur_entry.start_time, cur_entry.end_time);
			time_t temp_end_tm=tm_clamp(end_tm, cur_entry.start_time, cur_entry.end_time);
			duration+=temp_end_tm-temp_start_tm;
		}
	}
	return duration;
}

void draw_durations(int row, int col,t_log* a_log, statConfig* stat_conf, int stat_pos){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);
	time_t local_time=(unsigned long)time(NULL);
	time_t secs_in_day=24*60*60;
	int cell_w=8;
	int tag_w=12;
	if(col+tag_w>=max_col)
		return;
	int day_count= (max_col-col-tag_w)/cell_w-1;
	day_count= day_count > 7 ? 7 : day_count;

	int start_row=row;
	for(int i=0;i<stat_conf->count;i++){
		int tag_id=stat_conf->stat_colors[i].tag;
		char* tag_str=get_name_from_id(a_log, tag_id);

		int color=0;
		if(stat_conf!=0)
			color=get_tag_color_pair(&stat_conf->stat_colors[i].tag, stat_conf);
		attron(COLOR_PAIR(color));
		

		time_t last_midnight=local_time-((local_time+4*60*60)%(24*60*60));
		for(int j=0;j<=day_count;j++){
			int stat_pos_div=stat_pos/cell_w;
			time_t start_time=last_midnight - secs_in_day*j-stat_pos_div*secs_in_day;
			time_t end_time=last_midnight - secs_in_day*(j-1)-stat_pos_div*secs_in_day;
			if(row==start_row+1){
				attroff(COLOR_PAIR(color));
				mvftime_print(start_row-2,col+tag_w+cell_w*j-stat_pos%cell_w, "%e %h", start_time);
				print_str_n_times(start_row-2, col," ", tag_w);
				attron(COLOR_PAIR(color));
			}
			move(row-1,col+tag_w+cell_w*j-stat_pos%cell_w);
			printw("|");
			hline(' ',cell_w);
			print_duration(get_duration_in_range(a_log,tag_id,start_time,end_time));

		}
		move(row-1,col);
		if(tag_str[0]!=0){
			hline(' ',tag_w);
			printw("%s: ",tag_str);
		}
		attroff(COLOR_PAIR(color));
		row++;
	}
}


void grahp(int row, int col,t_log* a_log, statConfig* stat_conf, int stat_pos){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);

	time_t local_time=(unsigned long)time(NULL);
	time_t secs_in_day=24*60*60;
	char temp_str[MAX_NAME_SIZE];
	int cell_w=8;
	int tag_w=12;
	if(col+tag_w>=max_col)
		return;

	int start_row=row;
	//for(int i=0;i<stat_conf->count;i++){

	int color=0;
	if(stat_conf!=0)
		color=get_tag_color_pair(&stat_conf->stat_colors[0].tag, stat_conf);
	attron(COLOR_PAIR(color));

	time_t last_midnight=local_time-((local_time+4*60*60)%(24*60*60));
	time_t graph_start=last_midnight - secs_in_day*18;
	time_t incr=secs_in_day;
	int day_count=16;
	for(int j=0;j<=day_count;j++){
		int stat_pos_div=stat_pos/cell_w;
		time_t start_time=last_midnight - secs_in_day*j-stat_pos_div*secs_in_day;
		time_t end_time=last_midnight - secs_in_day*(j-1)-stat_pos_div*secs_in_day;
		time_t dur=get_duration_in_range(a_log, stat_conf->stat_colors[0].tag,start_time,end_time);
		for(int k=0;k<5;k++){
			int y=start_row-(int)(dur/60/20);
			while(y!=start_row){
				mvprintw(y,col+j+k," ");
				y++;
			}
			if(k==4)
				col+=k;
		}
		//mvprintw(start_row-(int)(dur/incr))+1,col+j,"%d",dur/60);
		if((col+j )>max_col)
			break;
	}
	attroff(COLOR_PAIR(color));
	row++;
	//}
}

