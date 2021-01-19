#include <ncurses.h>
#include "logs.h"

void print_normal_time(int row,int col,time_t tim){
	tm* broken_down_time=localtime(&tim);
	mvprintw(row,col,"%02d:%02d",
			broken_down_time->tm_hour,
			broken_down_time->tm_min); 
}

void print_duration(int duration){
	if(duration/60/60>0)
		printw("%dh ",duration/60/60);
	if(duration/60>0)
		printw("%dm",duration/60%60);
}

void draw_time_boxes(t_log* logp,int col_p,time_t cell_tm, int cell_minutes,int cur_row){
	time_t next_cell_tm=cell_tm+(cell_minutes*60);
	time_t local_time=(unsigned long)time(0);
	int col=20;
	if(cell_tm<local_time && next_cell_tm > local_time){
			mvprintw(cur_row, col+6+col, "<-- now");
	}
	for(int i=logp->index-1;i>=0;i--){
		time_t start_tm=logp->entries[i].start_time;
		time_t end_time=logp->entries[i].end_time;
		if(end_time < next_cell_tm && end_time > cell_tm){
			log_entry* entry=&logp->entries[i];
			mvprintw(cur_row, col+col_p, "=---->");
			printw("%s ",entry->name);

			attron(COLOR_PAIR(1));
			print_duration(entry->end_time-entry->start_time);
			attroff(COLOR_PAIR(1));
			break;
		}else if(end_time==0 &&  next_cell_tm > local_time && cell_tm < local_time ){
			log_entry* entry=&logp->entries[logp->index-1];
			mvprintw(cur_row, col+col_p, "++++++");
			printw("%s ",entry->name);

			attron(COLOR_PAIR(1));
			print_duration(local_time-entry->start_time);
			attroff(COLOR_PAIR(1));
			break;
		}else if(((next_cell_tm>end_time || end_time==0 )&& cell_tm < local_time) && cell_tm>start_tm){
			mvprintw(cur_row, col+col_p, "|    |");
			break;
		}else{
			//printw("%d,%d,%d;",(next_cell_tm<end_time || end_time==0 ),cell_tm < local_time,cell_tm>start_tm);
		}
	}
}

void print_logs(t_log* log_p,int row,int col,int max_row,int max_col,int cell_minutes,time_t cursor_pos_tm){

	mvprintw(max_row/2+row,0+col,"______________________________________________________________________");
	int count=0;

	tm* broken_down_time=localtime(&cursor_pos_tm);
	time_t nexthour_timestamp=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));

	for(int i=max_row+row;i>=0;i--){
		time_t cell_tm=nexthour_timestamp-cell_minutes*60*count;
		tm* broken_down_cell_tm=localtime(&cell_tm);

		move(i,col);
		mvprintw(i,col,"%02d:%02d",broken_down_cell_tm->tm_hour,broken_down_cell_tm->tm_min); 
		if(broken_down_cell_tm->tm_min==0){
			mvprintw(i,6+col,"%02d",broken_down_cell_tm->tm_hour);
			if(broken_down_cell_tm->tm_hour==0)
				mvprintw(i,9+col,"%02d/%02d/%02d",broken_down_cell_tm->tm_mday,broken_down_cell_tm->tm_mon+1,broken_down_cell_tm->tm_year+1900);
		}
		draw_time_boxes(log_p,col,cell_tm,cell_minutes,i);

		count++;
	}

	//if(max_col>125)
	//for(int i=0;i<log_p->index;i++){
		//log_entry* entry=&log_p->entries[i];
		//print_normal_time(0+i,70,entry->start_time);
		//if(entry->end_time == 0) 
			//mvprintw(0+i,79,"now");
		//else 
			//print_normal_time(0+i,77,entry->end_time);
		//printw(" %s, %s\n",entry->name,entry->sub_name);
	//}
}
