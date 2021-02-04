#include <cstring>
#include <math.h>
#include <ncurses.h>
#include <assert.h>
#include <sys/types.h>

#include "logs.h"
#include "autocomp.h"
#include "draw.h"
#include "stats.h"
#include "trlg_string.h"

void print_str_n_times(int row,int col, char* str,int n){
	for(int i=0;i<n/strlen(str);i++){
		mvprintw(row, col+i*strlen(str), "%s",str);
	}
}

tm get_tm(time_t time_stamp){
	tm result;
	localtime_r(&time_stamp, &result);
	return result;
}

void print_normal_time(time_t tim){
	tm broken_down_time=get_tm(tim);
	printw(" %02d:%02d",
			broken_down_time.tm_hour,
			broken_down_time.tm_min); 
}
void print_normal_date_time(time_t tim){
	tm broken_down_time=get_tm(tim);
	//printw("%02d/%02d/%02d %02d:%02d",
			//broken_down_time.tm_mday,
			//broken_down_time.tm_mon+1,
			//broken_down_time.tm_year+1900,
			//broken_down_time.tm_hour,
			//broken_down_time.tm_min); 
	printw("%02d %02d:%02d",
			broken_down_time.tm_mday,
			broken_down_time.tm_hour,
			broken_down_time.tm_min); 
}

void print_duration(time_t duration){
	if(duration/60/60>0)
		printw("%luh ",duration/60/60);
	if(duration/60>0)
		printw("%lum",duration/60%60);
}

void align_right_duration(int row, int col,time_t duration){
	int hours=duration/60/60;
	int minutes=duration/60%60;

	if(minutes>0){
		if(minutes>9){
			mvprintw(row,col-3,"%lum",minutes);
		}else{
			mvprintw(row,col-2,"%lum",minutes);
			//col++;
		}
	}
	if(hours>0){
		if(hours>9)
			mvprintw(row,col-7,"%luh ",hours);
		else
			mvprintw(row,col-6,"%luh ",hours);
	}
}

int draw_time_decorations(int cur_row,int col_p,time_t cell_tm, int cell_minutes,time_t cursor_offset, time_t quantized_cursor_pos_tm, u_int32_t draw_mask,int width){
	time_t next_cell_tm=cell_tm+(cell_minutes*60);
	time_t next_cell_tm_2=cell_tm-(cell_minutes*60);
	time_t current_time=(unsigned long)time(0);
	int col=0;
	if(quantized_cursor_pos_tm-cursor_offset < next_cell_tm && quantized_cursor_pos_tm-cursor_offset >= cell_tm && draw_mask & DRAW_cursor){
		print_str_n_times(cur_row,col_p, "_", 70);
		//move(cur_row,max_col-6);
	}

	tm broken_down_cell_tm=get_tm(cell_tm);

	if(get_tm(cell_tm).tm_mday!=get_tm(next_cell_tm_2).tm_mday && draw_mask & DRAW_DAY_DIVIDER){
		print_str_n_times(cur_row, col_p, "-", width);
	}

	if(draw_mask & DRAW_hm ){
		//if(get_tm(cell_tm).tm_hour!=get_tm(next_cell_tm_2).tm_hour )
		if (broken_down_cell_tm.tm_min==0) 
			attron(COLOR_PAIR(4));
		mvprintw(cur_row,col_p ,"%02d:%02d",broken_down_cell_tm.tm_hour,broken_down_cell_tm.tm_min); 
		attroff(COLOR_PAIR(4));
		col+=6;
	}
	if(draw_mask & DRAW_h){
		if (broken_down_cell_tm.tm_min==0) {
			mvprintw(cur_row,col+col_p,"%02d",broken_down_cell_tm.tm_hour);
		}
		col+=3;
	}

	if(draw_mask & DRAW_DATE ){
		if(get_tm(cell_tm).tm_mday!=get_tm(next_cell_tm_2).tm_mday){
			mvprintw(cur_row,col_p+col,"%02d/%02d/%02d",
					broken_down_cell_tm.tm_mday,
					broken_down_cell_tm.tm_mon+1,
					broken_down_cell_tm.tm_year+1900);
		}
		col+=11;
	}

	if(cell_tm<=current_time && next_cell_tm > current_time && draw_mask & DRAW_NOW){
		mvprintw(cur_row,col_p+ width+1, "<-- now");
	}
	return col;
}

void print_week_day(int row,int col,time_t tm){
	int weekday=get_tm(tm).tm_wday;
	switch(weekday){
		case 0:
			mvprintw(row-1,col,"Sun");
			break;
		case 1:
			mvprintw(row-1,col,"Mon");
			break;
		case 2:
			mvprintw(row-1,col,"Tue");
			break;
		case 3:
			mvprintw(row-1,col,"Wen");
			break;
		case 4:
			mvprintw(row-1,col,"Thu");
			break;
		case 5:
			mvprintw(row-1,col,"Fri");
			break;
		case 6:
			mvprintw(row-1,col,"Sat");
			break;
	}
}

calcCellResult calc_cell(t_log* logp,time_t cell_tm, int cell_minutes, time_t mask_start_tm,time_t mask_end_tm){
	time_t next_cell_tm=cell_tm+(cell_minutes*60);
	time_t current_time=(unsigned long)time(0);
	time_t last_duration=0;
	bool find_longest_entry=false;

	calcCellResult result={0,0,0};
	log_entry* longest_entry;

	bool draw_this_cell= (mask_end_tm==0) || (cell_tm > mask_start_tm && cell_tm < mask_end_tm);

	if(!draw_this_cell && (cell_tm+cell_minutes*60) > mask_start_tm && (cell_tm+cell_minutes*60) < mask_end_tm){
		result.entry_part=5;
		result.next_cell_tm=cell_tm+cell_minutes*60;
		return result;
	}else if (!draw_this_cell && (cell_tm-cell_minutes*60) > mask_start_tm && (cell_tm-cell_minutes*60) < mask_end_tm){
		result.entry_part=6;
		return result;
	}

	if(draw_this_cell){
		for(int i=logp->index-1;i>=0;i--){
			time_t start_tm=logp->entries[i].start_time;
			time_t end_time=logp->entries[i].end_time;
			if(end_time <= next_cell_tm && end_time >= cell_tm){
				find_longest_entry=true;
				log_entry* entry=&logp->entries[i];
				if((entry->end_time-entry->start_time) > last_duration){
					last_duration=entry->end_time-entry->start_time;
					longest_entry=entry;
					continue;
				}
			}else if(end_time==0 &&  next_cell_tm >= current_time && cell_tm <= current_time ){
				result.entry_part=4;
				result.entry=&logp->entries[logp->index-1];
				return result;
			}else if(((next_cell_tm<=end_time || end_time==0 )&& cell_tm <= current_time) && cell_tm>=start_tm){
				result.entry_part=2;
				result.entry=&logp->entries[i];
				return result;
			}else if(start_tm>=cell_tm && start_tm<=next_cell_tm){
				result.entry_part=1;
			}
		}

		if(find_longest_entry){
			result.entry_part=3;
			result.entry=longest_entry;
			return result;
		}
	}
	result.entry_part=0;
	return result;
}

void draw_cell(int row, int col,calcCellResult result, int width, statConfig* stat_conf, bool hide_text){
	int entry_part=result.entry_part;
	log_entry* entry_p=result.entry;
	time_t next_cell_tm=result.next_cell_tm;

	if(entry_part==3){
		int color=0;
		if(stat_conf!=0)
			color=get_tag_color_pair(entry_p->sub_name, stat_conf);
		attron(COLOR_PAIR(color));
		print_str_n_times(row, col, " ", width);

		if(!hide_text){
			mvprintw(row, col, "=>");
			printw(" ");
			print_warp_str(row,col+3, entry_p->name,117);
			//print_warp_str(row,col+width/2-strlen(entry_p->name)/2, entry_p->name,117);
			align_right_duration(row,col+width,entry_p->end_time-entry_p->start_time);
		}
		attroff(COLOR_PAIR(color));
	}else if(entry_part==4){
		printw(" ");
		int color=0;
		if(stat_conf!=0)
			color=get_tag_color_pair(entry_p->sub_name, stat_conf);
		attron(COLOR_PAIR(color));
		print_str_n_times(row, col, " ", width);
		if(!hide_text){
			mvprintw(row, col, "++");
			printw(" %s",entry_p->name);
			align_right_duration(row,col+width,(unsigned long)time(0)-entry_p->start_time);
		}
		attroff(COLOR_PAIR(color));
	}else if(entry_part==2){
		int color=0;
		if(stat_conf!=0)
			color=get_tag_color_pair(entry_p->sub_name, stat_conf);
		attron(COLOR_PAIR(color));
		print_str_n_times(row, col, " ", width);
		if(color==0)
			mvprintw(row, col, "|");
		else
			mvprintw(row, col, " ");

		attroff(COLOR_PAIR(color));
	}else if(entry_part==1){
		mvprintw(row, col, "--");
	}else if(entry_part==5){
		print_str_n_times(row, col, "-", width);
		if(width>5){
			print_week_day(row,col,next_cell_tm);
			printw(" %d",get_tm(next_cell_tm).tm_mday);
		}
	}else if(entry_part==6){
		print_str_n_times(row, col, "-", width);
	}

}

void print_logs(t_log* log_p,int row,int col,int cell_minutes,time_t cursor_pos_tm,statConfig* stat_conf){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);

	time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));
	time_t cursor_offset=cell_minutes*60*(int(max_row/2));
	quantized_cursor_pos_tm+=cursor_offset;

	int count=0;
	for(int i=max_row+row;i>=0;i--){
		int width=65;
		time_t cell_tm=quantized_cursor_pos_tm-cell_minutes*60*count;
		int dec_width=draw_time_decorations(i, 0, cell_tm, cell_minutes, cursor_offset, quantized_cursor_pos_tm, INT32_MAX , width);

		int entry_part=0;
		calcCellResult C_res=calc_cell(log_p, cell_tm, cell_minutes, 0, 0);
		draw_cell(i, dec_width,C_res, width-dec_width, stat_conf, false);
		count++;
	}
}

void print_weeks(t_log* log_p,int cell_minutes,time_t cursor_pos_tm,statConfig* stat_conf,int width,bool hide_text){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);

	int offset=9;

	//int width=25;
	int space_between=2;
	int days_to_fit=(int)(max_col-offset)/(width+space_between)-1;
	int fudge_factor=0;
	if(days_to_fit>=0)
		fudge_factor=(int)(((max_col-offset-2)%((width+space_between)*(days_to_fit+1)))/(days_to_fit+1));
	if(fudge_factor<width)
		width+=fudge_factor;

	time_t secs_in_day=24*60*60;
	time_t time_zone_offset=4*60*60;
	//cursor_pos_tm+=secs_in_day;
	for(int j=0;j<=days_to_fit;j++){
		int day=days_to_fit-j;
		day-=days_to_fit/2;
		time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));
		time_t cursor_offset=cell_minutes*60*(int(max_row/2)+1);
		quantized_cursor_pos_tm+=cursor_offset- (day)*secs_in_day;

		time_t last_midnight=cursor_pos_tm-((cursor_pos_tm+time_zone_offset)%(24*60*60));
		int count=0;
		for(int i=max_row-2;i>=0;i--){
			time_t cell_tm=quantized_cursor_pos_tm-cell_minutes*60*count;
			int entry_part=0;
			calcCellResult C_res=calc_cell(log_p, cell_tm, cell_minutes,
					last_midnight-secs_in_day*(day),
					last_midnight-secs_in_day*(day-1));
			if(j==0){
				draw_time_decorations(i, 0, cell_tm, cell_minutes, 
						cursor_offset,
						quantized_cursor_pos_tm,
						0  |DRAW_hm,
						width+offset);
				draw_time_decorations(i, offset, cell_tm, cell_minutes, 
						cursor_offset,
						quantized_cursor_pos_tm,
						DRAW_cursor,
						width);
			}else{
				draw_time_decorations(i, j*(width+space_between)+offset, cell_tm,
						cell_minutes,
						cursor_offset,
						quantized_cursor_pos_tm,
						DRAW_cursor,
						width);
			}
			draw_cell(i, j*(width+space_between)+offset,C_res,width, stat_conf, false);
			count++;
		}
	}
}

void draw_status(u_int32_t* state){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);
	int row=max_row/2,col=max_col/2;
	//for(int i=-5;i<5;i++)
		//hline(' ',row+i);
	mvprintw(row+5,col,"plase vait... ");

	if(*state%4==0){
		mvprintw(row+5,col+15,"|");
	}else if(*state%4==1){
		mvprintw(row+5,col+15,"/");
	}else if(*state%4==2){
		mvprintw(row+5,col+15,"-");
	}else if(*state%4==3){
		mvprintw(row+5,col+15,"\\");
	}
	*state=*state+1;
	refresh();
}

void draw_error(char* msg){
	erase();
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);
	size_t msg_len=strlen(msg);
	for(int i=max_col/2-msg_len-1;i<=max_col/2+msg_len;i++){
		mvprintw(max_row/2-1, i, "*");
	}

	mvprintw(max_row/2, max_col/2-msg_len/2, msg);

	for(int i=max_col/2-msg_len-1;i<=max_col/2+msg_len;i++){
		mvprintw(max_row/2+1, i, "*");
	}
	refresh();

	while (getch() == ERR){
	}
	erase();
}

void dr_box(int row, int col, int width, int height){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);

	if(row+height>=max_row) height=max_row-row-1;
	if(col+width>=max_col) width=max_col-col-1;

	for(int i=row;i<row+height;i++)
		for(int j=col;j<col+width;j++){
			if(i==row || i==row+height-1){
				mvprintw(i, j, "*");
			}else if(j==col || j == col+ width-1){
				mvprintw(i, j, "|");
			}else{
				mvprintw(i, j, " ");
			}
		}
}

void dr_text_box(int row, int col, int width, int height, char* msg){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);
	size_t msg_len=strlen(msg);
	if((row + col + width + height) == 0){
		row=max_row/2-5;
		col=max_col/2-msg_len/2;
	}

	dr_box(row, col-1, msg_len +4, 5);
	mvprintw(row+2,col+1,"%s",msg);
}

void draw_server_status(u_int32_t state){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);
	erase();
	int row=max_row/2,col=max_col/2;
	for(int i=-5;i<5;i++)
		hline(' ',row+i);
	if(state==0)
		mvprintw(row+5,col-10,"stalling");
	else if(state ==1)
		mvprintw(row+5,col-10,"done.");
	refresh();

}
