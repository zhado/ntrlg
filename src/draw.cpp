#include <ncurses.h>
#include "logs.h"

void print_normal_time(time_t tim){
	tm* broken_down_time=localtime(&tim);
	printw(" %02d:%02d",
			broken_down_time->tm_hour,
			broken_down_time->tm_min); 
}

void print_duration(int duration){
	if(duration/60/60>0)
		printw("%dh ",duration/60/60);
	if(duration/60>0)
		printw("%dm",duration/60%60);
}

log_entry* draw_time_boxes(t_log* logp,int col_p,time_t cell_tm, int cell_minutes,int cur_row, time_t quantized_cursor_pos_tm){
	time_t next_cell_tm=cell_tm+(cell_minutes*60);
	time_t local_time=(unsigned long)time(0);
	int col=20;

	tm* broken_down_cell_tm=localtime(&cell_tm);

	move(cur_row,col_p);
	if(broken_down_cell_tm->tm_mday%2==0){
		mvprintw(cur_row,col_p+10," ////// "); 
	}
	mvprintw(cur_row,col_p,"%02d:%02d",broken_down_cell_tm->tm_hour,broken_down_cell_tm->tm_min); 
	
	if(broken_down_cell_tm->tm_min==0){
		mvprintw(cur_row,6+col_p,"%02d",broken_down_cell_tm->tm_hour);
		if(broken_down_cell_tm->tm_hour==0)
			mvprintw(cur_row,9+col_p,"%02d/%02d/%02d",
					broken_down_cell_tm->tm_mday,
					broken_down_cell_tm->tm_mon+1,
					broken_down_cell_tm->tm_year+1900);
	}

	if(cell_tm<local_time && next_cell_tm > local_time){
			mvprintw(cur_row, col+22+col_p, "<-- now");
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
			if(end_time>=quantized_cursor_pos_tm && end_time <=(quantized_cursor_pos_tm+cell_minutes*60)){
				return entry;
			}
			break;
		}else if(end_time==0 &&  next_cell_tm >= local_time && cell_tm <= local_time ){
			log_entry* entry=&logp->entries[logp->index-1];
			mvprintw(cur_row, col+col_p, "++++++");
			printw("%s ",entry->name);

			attron(COLOR_PAIR(1));
			print_duration(local_time-entry->start_time);
			attroff(COLOR_PAIR(1));
			break;
		}else if(((next_cell_tm<end_time || end_time==0 )&& cell_tm < local_time) && cell_tm>start_tm){
			mvprintw(cur_row, col+col_p, "|    |");
			break;
		}else if(start_tm>cell_tm && start_tm<next_cell_tm){
			log_entry* entry=&logp->entries[i];
			mvprintw(cur_row, col+col_p, "------");
			//printw("%s ",entry->name);
		}
	}
	return 0;
}

log_entry* print_logs(t_log* log_p,int row,int col,int max_row,int max_col,int cell_minutes,time_t cursor_pos_tm){
	log_entry* result=0;
	log_entry* current_entry=0;
	time_t cursor_offset=cell_minutes*max_row/2*60;
	cursor_pos_tm+=cursor_offset;

	mvprintw(max_row/2+row,0+col,"______________________________________________________________________");
	int count=0;

	time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));

	for(int i=max_row+row;i>=0;i--){
		time_t cell_tm=quantized_cursor_pos_tm-cell_minutes*60*count;

		current_entry=draw_time_boxes(log_p,col,cell_tm,cell_minutes,i,
				quantized_cursor_pos_tm-cursor_offset);
		if(current_entry!=0)result=current_entry;
		count++;
	}
	return result;
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
