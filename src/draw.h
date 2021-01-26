#ifndef DRAW_H 
#define DRAW_H
#include <time.h>

// 00000000
//    |||||-- saati da wuti
//    ||||-- gverdita saati
//    |||-- tarigi
//    ||-- cursor
//    |-- no blocks

#define DRAW_NO_BLOCKS 32
#define DRAW_DAY_DIVIDER 16
#define DRAW_DATE 8
#define DRAW_h 2
#define DRAW_hm 1
void print_str_n_times(int row,int col, char* ch,int n);
void print_duration(int duration);
void print_normal_time(time_t tim);
void print_normal_date_time(time_t tim);
tm get_tm(time_t time_stamp);
#endif
