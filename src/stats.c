#include <string.h>
#include <ncurses.h>
#include <math.h>

#include "logs.h"
#include "autocomp.h"
#include "draw.h"
#include "trlg_common.h"
#include "trlg_string.h"
#include "logs.h"
#include "stats.h"
#include "trlg.h"

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

int get_tag_color_pair(int* tags, statConfig* stat_conf,int index){
	int count=0;
	for(int i=0;;i++){
		if(tags[i]==0)
			break;
		int cur_tag=tags[i];
		for(int j=0;j<stat_conf->count;j++){
			if(stat_conf->stat_colors[j].tag==cur_tag){
				if(count==index){
					return stat_conf->stat_colors[j].pair_id;
				}
				count++;
			}
		}
	}
	return 0;
}

int get_entry_tag_count(log_entry* entry){
	int* tags=entry->tags;
	
	int i=0;
	for(;;i++){
		if(tags[i]==0)
			break;
	}
	return i;
}

void reconstruct_color(statColor color, wchar_t* str){
	short bg=color.bg;
	short fg=color.fg;
	if(bg!=-1){
		wcsncat(str,L"(",MAX_NAME_SIZE);
		swprintf(str+wcslen(str),MAX_NAME_SIZE,L"%d",bg);
		if(fg!=-1){
			swprintf(str+wcslen(str),MAX_NAME_SIZE,L" %d",fg);
		}
		wcsncat(str,L")",MAX_NAME_SIZE);
	}
}

void add_statcolor(statConfig* stat_conf,t_log* log_p, strPart prt, int index){

	if(prt.length==-1){
		prt.length=wcslen(prt.start);
	}

	statColor col={0,-1,-1,-1};
	int first_p=-1;
	int sec_p=-1;
	wchar_t* str=prt.start;

	int count=stat_conf->count;

	for(int i=0;i<prt.length;i++){
		if(str[i]=='(')
			first_p=i;
		else if(str[i]==')')
			sec_p=i;

		if(str[i]==',' && i < prt.length){
			prt.length=i;
		}
	}

	int len=0;
	if(first_p!=-1)
		len=first_p;
	else
		len=prt.length;

	col.tag=get_tag_id(log_p, (strPart){str,len});
	if (col.tag==-1 && index != -1) return;
	
	if(index == -1)
		for(int i=0;i<stat_conf->count;i++){
			if(stat_conf->stat_colors[i].tag==col.tag)
				return;
		}

	if(first_p !=-1 && sec_p !=-1){
		swscanf(str+first_p+1,L"%d %d",&col.bg,&col.fg);
		if(index==-1){
			col.pair_id=count+10;
		}else{
			col.pair_id=index+10;
		}
		init_pair(col.pair_id,col.fg ,col.bg);
	}else{
		col.pair_id=0;
	}

	if(index==-1){
		stat_conf->stat_colors[count]=col;
		stat_conf->count++;
	}else{
		stat_conf->stat_colors[index]=col;
	}
}

statConfig generate_stat_colors(t_log* log_p,wchar_t* str){
	statConfig conf;
	conf.count=0;
	conf.stat_selection=0;

	if(str[0]==0)
		return conf;

	for(int i=0;;i++){
		strPart prt=get_nth_strpart(str, ',', i);
		if(prt.length==0){
			break;
		}
		add_statcolor(&conf, log_p, prt,-1);
	}

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
	int max_col=getmaxx(stdscr);
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
		wchar_t* tag_str=get_str_from_id(a_log, tag_id);

		int color=0;
		if(stat_conf!=0)
			color=get_tag_color_pair(&stat_conf->stat_colors[i].tag, stat_conf,0);
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
			printw("%ls: ",tag_str);
		}
		attroff(COLOR_PAIR(color));
		if(i==stat_conf->stat_selection){
			mvprintw(row-1,col+cell_w*(day_count+1)+tag_w,"<---");
		}
		row++;
	}
}

void raster_line(int y1,int x1,int y2,int x2){
		int dx=x2-x1;
		int dy=y2-y1;
		float dist=sqrt(dx*dx+dy*dy);
		float tx=dx/dist;
		float ty=dy/dist;
		float my_x=x1;
		float my_y=y1;
		int prev_my_x=x1;
		int prev_my_y=y1;
		char chr=' ';
		for(int i=0;i<ceil(dist);i++){
			my_x+=tx;
			my_y+=ty;
			int delta_x=(int)my_x-prev_my_x;
			int delta_y=(int)my_y-prev_my_y;

			if(delta_x>=1 && delta_y<=-1)
				chr='/';
			else if(delta_x>=1 && delta_y>=1) 
				chr='\\';
			else if(delta_x>=1) 
				chr='_';
			else if(delta_y<=1 && delta_y!=0)
				chr='|';
				
			if(dy>=1)
				mvprintw(my_y, my_x, "%c",chr);
			else
				mvprintw(prev_my_y, prev_my_x, "%c",chr);

			prev_my_x=(int)my_x;
			prev_my_y=(int)my_y;
		}
}

void grahp(int row, int col,app_state* app, candlestickOpts* opts){
	t_log* a_log=&app->logs;
	statConfig* stat_conf=&app->stat_conf;
	int max_col=getmaxx(stdscr);

	time_t local_time=(unsigned long)time(NULL);
	time_t secs_in_day=24*60*60;
	int cell_w=8;
	int tag_w=12;
	if(col+tag_w>=max_col)
		return;

	int start_row=row;

	int color=0;
	if(stat_conf!=0)
		color=get_tag_color_pair(&stat_conf->stat_colors[0].tag, stat_conf,0);
	attron(COLOR_PAIR(color));

	time_t last_midnight=local_time-((local_time+4*60*60)%(24*60*60));
	int day_count=16;
	for(int j=0;j<=day_count;j++){
		int stat_pos_div=opts->x_offset/cell_w;
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
		if((col+j)>max_col)
			break;
	}
	attroff(COLOR_PAIR(color));
	row++;
}

