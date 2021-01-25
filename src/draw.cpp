#include <cstring>
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

void draw_time_boxes(t_log* logp,int cur_row,int col_p,time_t cell_tm, int cell_minutes, time_t quantized_cursor_pos_tm, u_int32_t draw_mask, time_t mask_start_tm, time_t mask_end_tm){
	time_t next_cell_tm=cell_tm+(cell_minutes*60);
	time_t next_cell_tm_2=cell_tm-(cell_minutes*60);
	time_t current_time=(unsigned long)time(0);
	time_t last_duration=0;
	int col=0;
	bool find_longest_entry=false;

	tm broken_down_cell_2=get_tm(next_cell_tm_2);
	tm broken_down_cell_tm=get_tm(cell_tm);

	if(get_tm(cell_tm).tm_mday!=get_tm(next_cell_tm_2).tm_mday && draw_mask & DRAW_DAY_DIVIDER){
		print_str_n_times(cur_row, col_p, "-", 70);
	}

	if(draw_mask & DRAW_hm ){
		mvprintw(cur_row,col_p ,"%02d:%02d",broken_down_cell_tm.tm_hour,broken_down_cell_tm.tm_min); 
		col+=6;
	}
	if(draw_mask & DRAW_h){
		if(broken_down_cell_tm.tm_min==0 ){
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
	move(cur_row,col_p);


	if(cell_tm<=current_time && next_cell_tm > current_time){
			mvprintw(cur_row, col+30+col_p, "<-- now");
	}

	log_entry* longest_entry;
	bool draw_this_cell= (mask_end_tm==0) || (cell_tm > mask_start_tm && cell_tm < mask_end_tm);
	if(draw_this_cell){
		for(int i=logp->index-1;i>=0;i--){
			time_t start_tm=logp->entries[i].start_time;
			time_t end_time=logp->entries[i].end_time;
			if(end_time < next_cell_tm && end_time > cell_tm){
				find_longest_entry=true;
				log_entry* entry=&logp->entries[i];
				mvprintw(cur_row, col+col_p, "=---->");

				if((entry->end_time-entry->start_time) > last_duration){
					last_duration=entry->end_time-entry->start_time;
					longest_entry=entry;
					continue;
				}
			}else if(end_time==0 &&  next_cell_tm >= current_time && cell_tm < current_time ){
				log_entry* entry=&logp->entries[logp->index-1];
				mvprintw(cur_row, col+col_p, "++++++");
				printw("%s ",entry->name);

				attron(COLOR_PAIR(1));
				print_duration(current_time-entry->start_time);
				attroff(COLOR_PAIR(1));
				break;
			}else if(((next_cell_tm<end_time || end_time==0 )&& cell_tm < current_time) && cell_tm>start_tm){
				mvprintw(cur_row, col+col_p, "|    |");
				break;
			}else if(start_tm>cell_tm && start_tm<next_cell_tm){
				log_entry* entry=&logp->entries[i];
				mvprintw(cur_row, col+col_p, "------");
				//printw("%s ",entry->name);
			}
		}

		if(find_longest_entry){
			//get
			int row=0,col=0;
			getyx(stdscr, row, col);
			print_warp_str(row,col, longest_entry->name,25 );
			//printw("%s",longest_entry->name);
			printw(" ");
			attron(COLOR_PAIR(1));
			print_duration(longest_entry->end_time-longest_entry->start_time);
			attroff(COLOR_PAIR(1));
		}
	}
}

void print_logs(t_log* log_p,int row,int col,int max_row,int max_col,int cell_minutes,time_t cursor_pos_tm){
	log_entry* current_entry=0;

	time_t local_time=(unsigned long)time(NULL);
	tm broken_down_time=get_tm(local_time);
	time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));
	time_t cursor_offset=cell_minutes*60*(int(max_row/2)+1);
	quantized_cursor_pos_tm+=cursor_offset;

	time_t last_midnight=local_time-broken_down_time.tm_hour*60*60-broken_down_time.tm_min*60-broken_down_time.tm_sec;
	if(max_col>COL_CUTOFF)
		print_str_n_times(max_row/2+row,0+col, "_", 70);
	//else
		//mvprintw(max_row/2+row,70,"<<<");
	int count=0;
	for(int i=max_row+row;i>=0;i--){
		time_t cell_tm=quantized_cursor_pos_tm-cell_minutes*60*count;
		draw_time_boxes(log_p,i,col,cell_tm,cell_minutes,quantized_cursor_pos_tm,0 | DRAW_DAY_DIVIDER,last_midnight,local_time);
		count++;
	}
}

