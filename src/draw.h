#ifndef DRAW_H 
#define DRAW_H
#include <sys/types.h>
#include <time.h>
#include "logs.h"

// 00000000
//   ||||||-- saati da wuti
//   |||||-- gverdita saati
//   ||||-- tarigi
//   |||-- cursor
//   ||-- now indicator 
//   |-- draw_cursor

#define DRAW_cursor 64
#define DRAW_NOW 32
#define DRAW_DAY_DIVIDER 16
#define DRAW_DATE 8
#define DRAW_h 2
#define DRAW_hm 1

struct calcCellResult{
	//entry_part = 0 empty, 1 start, 2 body, 3 ended entry end, 4 ongoing entry end, 5 day start, 6 day end
	int entry_part;
	int entry_h;
	log_entry* entry;
	time_t next_cell_tm;
};

void print_str_n_times(int row,int col, char* ch,int n);
void print_duration(int duration);
void print_normal_time(time_t tim);
void print_normal_date_time(time_t tim);
void mvftime_print(int row, int col, char* format, time_t Time);
void ftime_print(char* format, time_t Time);
tm get_tm(time_t time_stamp);
void* draw_status(void * args, int packet_counter);
void draw_error(char *msg);
#endif
