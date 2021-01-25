#ifndef DRAW_H 
#define DRAW_H
#include <time.h>

// 00000000
//     ||||-- saati da wuti
//     |||-- gverdita saati
//     ||-- tarigi
//     |-- cursor
void print_str_n_times(int row,int col, char* ch,int n);
void print_duration(int duration);
void print_normal_time(time_t tim);
void print_normal_date_time(time_t tim);
tm get_tm(time_t time_stamp);
#endif
