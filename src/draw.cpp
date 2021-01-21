#include <cstring>
#include <ncurses.h>
#include "logs.h"

void print_str_n_times(int row,int col, char* str,int n){
	for(int i=0;i<n/strlen(str);i++){
		mvprintw(row, col+i*strlen(str), "%s",str);
	}
}

void print_normal_time(time_t tim){
	tm* broken_down_time=localtime(&tim);
	printw(" %02d:%02d",
			broken_down_time->tm_hour,
			broken_down_time->tm_min); 
}
void print_normal_date_time(time_t tim){
	tm* broken_down_time=localtime(&tim);
	printw("%02d/%02d/%02d %02d:%02d",
			broken_down_time->tm_mday,
			broken_down_time->tm_mon+1,
			broken_down_time->tm_year+1900,
			broken_down_time->tm_hour,
			broken_down_time->tm_min); 
}

void print_duration(time_t duration){
	if(duration/60/60>0)
		printw("%luh ",duration/60/60);
	if(duration/60>0)
		printw("%lum",duration/60%60);
}

int get_day(time_t tim){
	tm* broken_down_time=localtime(&tim);
	return broken_down_time->tm_mday;
}

log_entry* draw_time_boxes(t_log* logp,int cur_row,int col_p,time_t cell_tm, int cell_minutes, time_t quantized_cursor_pos_tm){
	time_t next_cell_tm=cell_tm+(cell_minutes*60);
	time_t next_cell_tm_2=cell_tm-(cell_minutes*60);
	time_t local_time=(unsigned long)time(0);
	time_t last_duration=0;
	int col=20;
	bool find_longest_entry=false;

	tm* broken_down_cell_2=localtime(&next_cell_tm_2);
	int day2=broken_down_cell_2->tm_mday;
	tm* broken_down_cell_tm=localtime(&cell_tm);
	int day1=broken_down_cell_tm->tm_mday;
	if(day1!=day2){
		//print_str_n_times(cur_row, col_p, "-", 30);
		mvprintw(cur_row,9+col_p,"%02d/%02d/%02d",
				broken_down_cell_tm->tm_mday,
				broken_down_cell_tm->tm_mon+1,
				broken_down_cell_tm->tm_year+1900);
	}
	move(cur_row,col_p);

	mvprintw(cur_row,col_p,"%02d:%02d",broken_down_cell_tm->tm_hour,broken_down_cell_tm->tm_min); 
	if(broken_down_cell_tm->tm_min==0)
		mvprintw(cur_row,6+col_p,"%02d",broken_down_cell_tm->tm_hour);

	if(cell_tm<local_time && next_cell_tm > local_time){
			mvprintw(cur_row, col+22+col_p, "<-- now");
	}

	log_entry* longest_entry;
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
			}
			continue;
		}else if(end_time==0 &&  next_cell_tm >= local_time && cell_tm < local_time ){
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

	if(find_longest_entry){
		printw("%s ",longest_entry->name);
		attron(COLOR_PAIR(1));
		print_duration(longest_entry->end_time-longest_entry->start_time);
		attroff(COLOR_PAIR(1));
		if(longest_entry->end_time>=quantized_cursor_pos_tm && longest_entry->end_time <=(quantized_cursor_pos_tm+cell_minutes*60)){
			return longest_entry;
		}
	}

	//printw("      %u %u",br_2->tm_mday,broken_down_cell_tm->tm_mday);
	return 0;
}

log_entry* print_logs(t_log* log_p,int row,int col,int max_row,int max_col,int cell_minutes,time_t cursor_pos_tm){
	log_entry* result=0;
	log_entry* current_entry=0;

	time_t cursor_offset=cell_minutes*max_row/2*60;
	cursor_pos_tm+=cursor_offset;

	print_str_n_times(max_row/2+row,0+col, "_", 70);
	int count=0;

	time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));

	for(int i=max_row+row;i>=0;i--){
		time_t cell_tm=quantized_cursor_pos_tm-cell_minutes*60*count;

		current_entry=draw_time_boxes(log_p,i,col,cell_tm,cell_minutes,quantized_cursor_pos_tm-cursor_offset);
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
