#include <cstring>
#include <math.h>
#include <ncurses.h>
#include <sys/types.h>
#include "logs.h"
#include "autocomp.h"
#include "draw.h"
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
		mvprintw(cur_row,col_p+ width-sizeof("<-- now"), "<-- now");
	}
	return col;
}

void print_week_day(int row,int col,time_t tm,int cell_minutes){
	int weekday=get_tm(tm+cell_minutes*60).tm_wday;
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

void draw_time_boxes(t_log* logp,int cur_row,int col_p,time_t cell_tm, int cell_minutes, time_t mask_start_tm,time_t mask_end_tm,int width){
	time_t next_cell_tm=cell_tm+(cell_minutes*60);
	time_t next_cell_tm_2=cell_tm-(cell_minutes*60);
	time_t current_time=(unsigned long)time(0);
	time_t last_duration=0;
	bool find_longest_entry=false;

	log_entry* longest_entry;

	bool draw_this_cell= (mask_end_tm==0) || (cell_tm > mask_start_tm && cell_tm < mask_end_tm);
	if(!draw_this_cell && (cell_tm+cell_minutes*60) > mask_start_tm && (cell_tm+cell_minutes*60) < mask_end_tm){
		print_str_n_times(cur_row, col_p, "-", width);
		print_week_day(cur_row,col_p,cell_tm+cell_minutes*60,cell_minutes);
		printw(" %d",get_tm(cell_tm+cell_minutes*60).tm_mday);
	}else if (!draw_this_cell && (cell_tm-cell_minutes*60) > mask_start_tm && (cell_tm-cell_minutes*60) < mask_end_tm){
		print_str_n_times(cur_row, col_p, "-", width);
	}

	if(draw_this_cell){
		for(int i=logp->index-1;i>=0;i--){
			time_t start_tm=logp->entries[i].start_time;
			time_t end_time=logp->entries[i].end_time;
			if(end_time < next_cell_tm && end_time > cell_tm){
				find_longest_entry=true;
				log_entry* entry=&logp->entries[i];
				mvprintw(cur_row, col_p, "=---->");
				if((entry->end_time-entry->start_time) > last_duration){
					last_duration=entry->end_time-entry->start_time;
					longest_entry=entry;
					continue;
				}
			}else if(end_time==0 &&  next_cell_tm >= current_time && cell_tm < current_time ){
				log_entry* entry=&logp->entries[logp->index-1];
				mvprintw(cur_row, col_p, "++++++");
				//printw(" ");
				printw("%s ",entry->name);
				attron(COLOR_PAIR(1));
				print_duration(current_time-entry->start_time);
				attroff(COLOR_PAIR(1));
				break;
			}else if(((next_cell_tm<end_time || end_time==0 )&& cell_tm < current_time) && cell_tm>start_tm){
				log_entry* entry=&logp->entries[i];
				mvprintw(cur_row, col_p, "|    |");
				break;
			}else if(start_tm>cell_tm && start_tm<next_cell_tm){
				log_entry* entry=&logp->entries[i];
				mvprintw(cur_row, col_p, "------");
			}
		}

		if(find_longest_entry){
			printw(" ");
			//get
			int row=0,col=0;
			getyx(stdscr, row, col);
			print_warp_str(row,col, longest_entry->name,width-13);
			//printw("%s",longest_entry->name);
			printw(" ");
			attron(COLOR_PAIR(1));
			print_duration(longest_entry->end_time-longest_entry->start_time);
			attroff(COLOR_PAIR(1));
		}
	}
}

void print_logs(t_log* log_p,int row,int col,int cell_minutes,time_t cursor_pos_tm){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);
	log_entry* current_entry=0;

	time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));
	time_t cursor_offset=cell_minutes*60*(int(max_row/2));
	quantized_cursor_pos_tm+=cursor_offset;

	int count=0;
	for(int i=max_row+row;i>=0;i--){
		time_t cell_tm=quantized_cursor_pos_tm-cell_minutes*60*count;
		int dec_width=draw_time_decorations(i, 0, cell_tm, cell_minutes, cursor_offset, quantized_cursor_pos_tm, INT32_MAX , 70);
		draw_time_boxes(log_p,i,col+dec_width,cell_tm,cell_minutes,0,0,70);
		count++;
	}
}

void print_weeks(t_log* log_p,int cell_minutes,time_t cursor_pos_tm){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);

	log_entry* current_entry=0;

	int offset=9;

	int width=25;
	int space_between=2;
	int days_to_fit=(int)(max_col-offset)/(width+space_between)-1;
	int fudge_factor=0;
	if(days_to_fit>=0)
		fudge_factor=(int)(((max_col-offset-2)%((width+space_between)*(days_to_fit+1)))/(days_to_fit+1));
	if(fudge_factor<width)
		width+=fudge_factor;

	for(int j=0;j<=days_to_fit;j++){
		int day=days_to_fit-j;
		time_t local_time=(unsigned long)time(NULL);
		time_t secs_in_day=24*60*60;
		tm broken_down_time=get_tm(local_time);
		time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));
		time_t cursor_offset=cell_minutes*60*(int(max_row/2)+1);
		quantized_cursor_pos_tm+=cursor_offset- day*secs_in_day;
		//time_t prefered_time_offset=-16*60*60;
		time_t prefered_time_offset=0;

		time_t last_midnight=cursor_pos_tm-(cursor_pos_tm%(24*60*60));
		int count=0;
		for(int i=max_row-2;i>=0;i--){
			time_t cell_tm=quantized_cursor_pos_tm-cell_minutes*60*count;
			time_t cur_end_day=last_midnight-secs_in_day*(day);
			if(j==0){
				draw_time_decorations(i, 0, cell_tm-prefered_time_offset, cell_minutes, 
						cursor_offset,
						quantized_cursor_pos_tm,
						0  |DRAW_hm,
						width+offset);
				draw_time_decorations(i, offset, cell_tm, cell_minutes, 
						cursor_offset,
						quantized_cursor_pos_tm,
						DRAW_cursor,
						width);
				draw_time_boxes(log_p,i,offset,cell_tm-prefered_time_offset,cell_minutes,	
						last_midnight-secs_in_day*(day),
						last_midnight-secs_in_day*(day-1),
						width);
			}else{
				draw_time_decorations(i, j*(width+space_between)+offset, cell_tm,
						cell_minutes,
						cursor_offset,
						quantized_cursor_pos_tm,
						 (day==0)*DRAW_NOW+ DRAW_cursor,
						width);
				draw_time_boxes(log_p,i,j*(width+space_between)+offset,cell_tm-prefered_time_offset
						,cell_minutes,	
						last_midnight-secs_in_day*(day),
						last_midnight-secs_in_day*(day-1),
						width);
			}
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
