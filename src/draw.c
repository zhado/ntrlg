#include <string.h>
#include <math.h>
#include <ncurses.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

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

void print_str_n_timesw(char* str,int n){
	for(int i=0;i<n/strlen(str);i++){
		printw( "%s",str);
	}
}

struct tm get_tm(time_t time_stamp){
	struct	tm result;
	localtime_r(&time_stamp, &result);
	return result;
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

time_t get_cursor_offset(int cell_minutes){
		int max_row=getmaxy(stdscr);
		return cell_minutes*60*((int)(max_row/2)+1);
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

	struct tm broken_down_cell_tm=get_tm(cell_tm);

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

void mvftime_print(int row, int col, char* format, time_t Time){
	struct tm timeinfo=get_tm(Time);
	char week_str[MAX_NAME_SIZE];
	strftime(week_str, MAX_NAME_SIZE,format, &timeinfo);
	mvprintw(row, col, "%s", week_str);
}

void ftime_print(char* format, time_t Time){
	struct tm timeinfo=get_tm(Time);
	char week_str[MAX_NAME_SIZE];
	strftime(week_str, MAX_NAME_SIZE,format, &timeinfo);
	printw("%s", week_str);
}

calcCellResult calc_cell(t_log* logp,time_t cell_tm, int cell_minutes, time_t mask_start_tm,time_t mask_end_tm){
	time_t next_cell_tm=cell_tm+(cell_minutes*60);
	time_t prev_cell_tm=cell_tm-(cell_minutes*60);
	time_t current_time=(unsigned long)time(0);
	time_t last_duration=0;
	bool find_longest_entry=false;

	calcCellResult result={0,0,0};
	log_entry* longest_entry;

	bool draw_this_cell= (mask_end_tm==0) || (cell_tm > mask_start_tm && cell_tm < mask_end_tm);

	if(!draw_this_cell && next_cell_tm > mask_start_tm && next_cell_tm < mask_end_tm){
		result.entry_part=5;
		result.next_cell_tm=next_cell_tm;
		return result;
	}else if (!draw_this_cell && prev_cell_tm > mask_start_tm && prev_cell_tm < mask_end_tm){
		result.entry_part=6;
		return result;
	}

	if(!draw_this_cell){
		result.entry_part=0;
		return result;
	}

	for(int i=logp->index-1;i>=0;i--){
		time_t start_tm=logp->entries[i].start_time;
		time_t end_time=logp->entries[i].end_time;
		if(end_time < next_cell_tm && end_time >= cell_tm){
			find_longest_entry=true;
			log_entry* entry=&logp->entries[i];
			if((entry->end_time-entry->start_time) > last_duration){
				last_duration=entry->end_time-entry->start_time;
				longest_entry=entry;
				continue;
			}
		}else if(end_time==0 &&  next_cell_tm > current_time && cell_tm <= current_time ){
			// current entry ++
			result.entry_part=4;
			result.entry=&logp->entries[logp->index-1];
			return result;
		}else if(((next_cell_tm<=end_time || end_time==0 )&& cell_tm < current_time) && cell_tm>=start_tm){
			// entry body |
			result.entry_part=2;
			result.entry=&logp->entries[i];
			return result;
		}else if(start_tm>cell_tm && start_tm<=next_cell_tm){
			// entry start -
			result.entry_part=1;
			result.entry=&logp->entries[i];
		}else if(find_longest_entry){
			// ended entry end 'name'
			result.entry_part=3;
			result.entry=longest_entry;
			return result;
		}
	}

	return result;
}

void draw_cell(int row, int col,calcCellResult result, int width, statConfig* stat_conf, bool hide_text,int drag_status, log_entry* entry_under_cursor){
	int entry_part=result.entry_part;
	log_entry* entry_p=result.entry;
	time_t next_cell_tm=result.next_cell_tm;

	if(drag_status!=0){
		assert(entry_under_cursor!=0);
	}

	if(entry_part==3){
		int color=0;
		if(stat_conf!=0)
			color=get_tag_color_pair(entry_p->tags, stat_conf,0);
		attron(COLOR_PAIR(color));
		print_str_n_times(row, col, " ", width);

		if(!hide_text){
			if(width > 2 )
				mvprintw(row, col, "=> ");
			//print_warp_str(row,col+3, entry_p->name,117);
			print_chopoff(row,col+3, entry_p->name,width-10);
			if(width > 7 )
				align_right_duration(row,col+width,entry_p->end_time-entry_p->start_time);
		}
		attroff(COLOR_PAIR(color));

		if(drag_status==3 && entry_under_cursor==result.entry){
			mvprintw(row, col+width, "DD");
			mvprintw(row, col-2, "DD");
		}

	}else if(entry_part==4){
		printw(" ");
		int color=0;
		if(stat_conf!=0)
			color=get_tag_color_pair(entry_p->tags, stat_conf,0);
		attron(COLOR_PAIR(color));
		print_str_n_times(row, col, " ", width);
		if(!hide_text){
			if(width > 2 )
				mvprintw(row, col, "++");
			print_chopoff(row,col+3, entry_p->name,width-10);
			if(width > 7 )
			align_right_duration(row,col+width,(unsigned long)time(0)-entry_p->start_time);
		}
		attroff(COLOR_PAIR(color));
		if(drag_status==3 && entry_under_cursor==result.entry){
			mvprintw(row, col+width, "DD");
			mvprintw(row, col-2, "DD");
		}
	}else if(entry_part==2){
		int color=0;
		int tag_count=get_entry_tag_count(entry_p);
		int divided_w=width/(tag_count == 0 ? 1 : tag_count);
		if(divided_w==0){
			divided_w=1;
		}

		for(int i=0;i<tag_count;i++){
			//if(i==tag_count-1)
				//divided_w+=width%divided_w;
			if(stat_conf!=0)
				color=get_tag_color_pair(entry_p->tags, stat_conf,i);
			attron(COLOR_PAIR(color));
			print_str_n_times(row, col+divided_w*i, " ", divided_w+width%divided_w);
			if(color==0)
				mvprintw(row, col+divided_w*i, "|");

			attroff(COLOR_PAIR(color));
		}

		if(drag_status==2 && entry_under_cursor==result.entry){
			mvprintw(row, col+width, "DD");
			mvprintw(row, col-2, "DD");
		}
	}else if(entry_part==1){
		mvprintw(row, col, "--");
		if(drag_status==1 && entry_under_cursor==result.entry){
			mvprintw(row, col+width, "DD");
			mvprintw(row, col-2, "DD");
		}

	}else if(entry_part==5){
		print_str_n_times(row, col, "-", width);
		if(width>5){
			mvftime_print(row-1, col, "%e %h",next_cell_tm);
		}
	}else if(entry_part==6){
		print_str_n_times(row, col, "-", width);
	}

}

void print_logs(t_log* log_p,int row,int col,int cell_minutes,time_t cursor_pos_tm,statConfig* stat_conf,window_state win_state, log_entry* entry_under_cursor){
	int max_row=getmaxy(stdscr);
	int drag_status=0;
	switch (win_state) {
		case entry_start_resize:{
			drag_status=1;
		}break;
		case entry_body_resize:{
			drag_status=2;
		}break;
		case entry_end_resize:{
			drag_status=3;
		}break;
		default:{
			drag_status=0;
		}
	}

	time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));
	time_t cursor_offset=cell_minutes*60*((int)(max_row/2));
	quantized_cursor_pos_tm+=cursor_offset;

	int count=0;
	for(int i=max_row+row;i>=0;i--){
		int width=65;
		time_t cell_tm=quantized_cursor_pos_tm-cell_minutes*60*count;
		int dec_width=draw_time_decorations(i, 0, cell_tm, cell_minutes, cursor_offset, quantized_cursor_pos_tm, INT32_MAX , width);

		calcCellResult C_res=calc_cell(log_p, cell_tm, cell_minutes, 0, 0);
		draw_cell(i, dec_width,C_res, width-dec_width, stat_conf, false, drag_status, entry_under_cursor);
		count++;
	}
}

void print_weeks(t_log* log_p,int cell_minutes,time_t cursor_pos_tm,statConfig* stat_conf,int width,bool hide_text, bool fudge_toggle){
	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);

	int offset=9;

	//int width=25;
	int space_between=1;
	int days_to_fit=(int)(max_col-offset)/(width+space_between)-1;
	int fudge_factor=0;

	if(fudge_toggle){
		if(days_to_fit>=0)
			fudge_factor=(int)(((max_col-offset-2)%((width+space_between)*(days_to_fit+1)))/(days_to_fit+1));
		if(fudge_factor<width)
			width+=fudge_factor;
	}

	time_t cursor_offset=get_cursor_offset(cell_minutes);
	time_t secs_in_day=24*60*60;
	time_t time_zone_offset=4*60*60;
	//cursor_pos_tm+=secs_in_day;
	for(int j=0;j<=days_to_fit;j++){
		int day=days_to_fit-j;
		day-=days_to_fit/2;
		time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));
		quantized_cursor_pos_tm+=cursor_offset- (day)*secs_in_day;

		time_t last_midnight=cursor_pos_tm-((cursor_pos_tm+time_zone_offset)%(24*60*60));
		int count=0;
		//mvprintw(0,j*(width+space_between)+offset,"%d %d",j,day);
		for(int i=max_row-2;i>=0;i--){
			time_t cell_tm=quantized_cursor_pos_tm-cell_minutes*60*count;
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

			draw_cell(i, j*(width+space_between)+offset,C_res,width, stat_conf, hide_text,0,0);
			count++;
		}
		if(day==0){
			move(0,j*(width+space_between)+offset-1);
			vline('|',max_col);
			move(0,j*(width+space_between)+offset+width);
			vline('|',max_col);
		}
	}
}

void draw_status(int* counter, int packet_counter){

	int max_row,max_col;
	getmaxyx(stdscr,max_row,max_col);
	int row=max_row/2,col=max_col/2;
	mvprintw(row+6,col,"%d recieved",packet_counter);
	mvprintw(row+5,col,"plase vait... ");
	if(*counter%4==0){
		mvprintw(row+5,col+15,"|");
	}else if(*counter%4==1){
		mvprintw(row+5,col+15,"/");
	}else if(*counter%4==2){
		mvprintw(row+5,col+15,"-");
	}else if(*counter%4==3){
		mvprintw(row+5,col+15,"\\");
	}
	//fprintf(stderr,"-----------%d\n",counter);
	*counter=*counter+1;
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

bool draw_yn_prompt(char* msg){
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
	int chr=0;

	while(chr != 'y' && chr != 'n' && chr != 'Y' && chr != 'N'){
		chr=getch();
	}
	bool are_you_sure_result = (chr == 'y' || chr == 'Y') ? 1 : 0;
	erase();

	return are_you_sure_result;
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
