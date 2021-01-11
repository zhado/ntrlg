#include <stdio.h>
#include <time.h>
#include <linux/types.h>
#include <sys/types.h>
#include <ncurses.h>
typedef unsigned long ul64;

const int max_name_size=100;

struct entry {
	char* name;
	char* sub_name;
	time_t start_time;
	time_t end_time;
};

void print_normal_time(time_t* tim){
	tm* broken_down_time=localtime(tim);
	fprintf(stdout, "%d/%d/%d %d:%d\n",
			broken_down_time->tm_mday,
			broken_down_time->tm_mon+1,
			broken_down_time->tm_year+1900,
			broken_down_time->tm_hour,
			broken_down_time->tm_min); 
}

int main(){
	time_t epoch_time=(unsigned long)time(NULL);
	//print_normal_time(&epoch_time);

	initscr();			/* Start curses mode 		  */
	printw("Hello World !!!");	/* Print Hello World		  */
	refresh();			/* Print it on to the real screen */
	getch();			/* Wait for user input */
	endwin();			/* End curses mode		  */

	return 0;
	
}
