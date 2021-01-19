#include <stdio.h>
#include <time.h>
#include <linux/types.h>
#include <sys/types.h>
#include <ncurses.h>
#include <cstdlib>
#include <math.h>
#include <string.h>

#include "main.h"
#include "autocomp.cpp"
#include "draw.cpp"
#include "logs.h"

bool UNSAVED_CHANGES=false;

enum window_state {
	view,logging
};

char char_at(int row,int col){
	chtype a=mvinch(row,col);
	char ret= A_CHARTEXT & a;
	return ret;
}

void end_last_entry(t_log* log_p){
	UNSAVED_CHANGES=true;
	log_entry* entry=&log_p->entries[log_p->index-1];
	if(entry->start_time==0){
		fprintf(stderr, "entry not started.");
	}else if(entry->end_time==0){
		entry->end_time=(unsigned long)time(0);
	}else{
		UNSAVED_CHANGES=false;
	}
}

void remove_commas_from_end(char* str){
	while(str[strlen(str)-1]==','){
		str[strlen(str)-1]=0;
	}
}

void append_entry(t_log* log_p, char* name, char* sub_name,time_t start_time,time_t end_time){
	UNSAVED_CHANGES=true;
	if(log_p->index!=0)
		end_last_entry(log_p);
	if(log_p->allocated < (log_p->index+1) ){
		log_p->entries=(log_entry*)realloc(log_p->entries, sizeof(log_entry)*(log_p->allocated+REALLOC_INCREMENT));
		log_p->allocated=log_p->allocated+REALLOC_INCREMENT;
	}
	log_entry* entry=&log_p->entries[log_p->index];

	entry->name=(char*)calloc(sizeof(char)*MAX_NAME_SIZE,1);
	entry->sub_name=(char*)calloc(sizeof(char)*MAX_NAME_SIZE,1);
	entry->end_time=end_time;

	entry->start_time=start_time;
	strcpy(entry->name, name);
	remove_spaces(sub_name);
	remove_commas_from_end(sub_name);
	strcpy(entry->sub_name, sub_name);
	log_p->index++;
}

t_log* load_log(const char* file_name){
	FILE* fp=fopen(file_name,"r");
	t_log* a_log=(t_log*)malloc(sizeof(t_log));
	a_log->allocated=0;
	a_log->index=0;
	a_log->entries=0;

	char line[400];
	int line_index=0;
	while (fgets(line,400,fp)!=0){
		int quotes[4]={0,0,0,0},index=0;
		char temp_name[MAX_NAME_SIZE];
		char temp_subname[MAX_NAME_SIZE];
		memset(temp_name, 0, MAX_NAME_SIZE);
		memset(temp_subname, 0, MAX_NAME_SIZE);
		time_t temp_start_time=0;
		time_t temp_end_time=0;
		for(int i=0;i<strlen(line);i++){
			if(line[i]=='"'){
				quotes[index++]=i;
			}
		}

		// TODO: sahinelebaa es
		if(quotes[1]!=0){
			memcpy(temp_name, line+quotes[0]+1,quotes[1]-quotes[0]-1);
		}
		if(quotes[3]!=0){
			memcpy(temp_subname, line+quotes[2]+1,quotes[3]-quotes[2]-1);
		}

		sscanf(line,"%lu %lu",&temp_start_time,&temp_end_time);
		remove_spaces(temp_subname);

		append_entry(a_log, temp_name, temp_subname, temp_start_time, temp_end_time);
		line_index++;
		memset(temp_name,0,MAX_NAME_SIZE);
		memset(temp_subname,0,MAX_NAME_SIZE);
	}
	a_log->index=line_index;

	fclose(fp);
	UNSAVED_CHANGES=false;
	return a_log;
}

void save_log(t_log* log_p, const char* file_name){
	UNSAVED_CHANGES=false;
	FILE* fp=fopen(file_name,"w");

	for(int i=0;i<log_p->index;i++){
		log_entry* entry=&log_p->entries[i];
		fprintf(fp, "%lu %lu \"%s\" \"%s\"\n",entry->start_time,entry->end_time,entry->name,entry->sub_name);
	}

	fclose(fp);
}

void free_log(t_log* log_p){
	for (int i=0;i<log_p->index;i++){
		log_entry* entry=&log_p->entries[i];
		free(entry->name);
		free(entry->sub_name);
	}
	free(log_p);
}

int main(){
	int cell_minutes=20;
	time_t cursor_pos_tm=(unsigned long)time(0);
	t_log* a_log=(t_log*)malloc(sizeof(t_log));

	a_log->index=0;
	a_log->entries=(log_entry*)malloc(sizeof(log_entry)*100);
	a_log->allocated=100;
	int max_row=0,max_col=0;
	window_state state=view;
	
	free_log(a_log);
	a_log=load_log(database_file);

	initscr();
	start_color();
	use_default_colors();
	init_pair(1, COLOR_GREEN, -1);
	init_pair(2, -1, COLOR_BLACK);
	init_pair(3, -1, COLOR_MAGENTA);
	cbreak();
	raw();
	set_escdelay(20);
	keypad(stdscr, TRUE);
	halfdelay(20);
	noecho();
	curs_set(0);

	int c=0;

	char name[MAX_NAME_SIZE];
	char sub_name[MAX_NAME_SIZE];
	memset(name,0,MAX_NAME_SIZE);
	memset(sub_name,0,MAX_NAME_SIZE);
	 //2=subname 1=name
	int logging_state=1;
	int log_selection=-1;
	bool append_log=false;
	bool change_log=false;
	match_result result;
	log_entry* entry_under_cursor=0;

	while(true){

		erase();
		getmaxyx(stdscr,max_row,max_col);
		if (c == 27){
			mvprintw(max_row-3,max_col-11,"esc pressed");
			memset(name,0,100);
			memset(sub_name,0,100);
			state=view;
			logging_state=1;
		}
		if(state==logging){
			if(c > 31 && c <=126){
				if(logging_state==1 && strlen(name) < MAX_NAME_SIZE){
					name[strlen(name)]=c;
				}else if ( logging_state ==2 && strlen(sub_name) < MAX_NAME_SIZE){
					sub_name[strlen(sub_name)]=c;
				}
			log_selection=-1;
			}else if (c == 263 || c==127){
				if(logging_state==1){
					name[strlen(name)-1]=0;
				}else{
					sub_name[strlen(sub_name)-1]=0;
				}
			}else if (c == KEY_UP){
				if(log_selection<result.match_count)
					log_selection++;
			}else if (c == KEY_DOWN){
				if(log_selection>-1)
					log_selection--;
			}else if (c == 10){
				if(logging_state==1){
					logging_state++;
				}else{
					if(log_selection==-1){
						if(append_log){
							end_last_entry(a_log);
							append_entry(a_log, name, sub_name,a_log->entries[a_log->index-1].end_time,0);
							append_log=false;
						}else if(change_log){
							strcpy(entry_under_cursor->name, name);
							strcpy(entry_under_cursor->sub_name, sub_name);
							change_log=false;
							UNSAVED_CHANGES=true;
						}else{
							append_entry(a_log, name, sub_name,(unsigned long)time(0),0);
						}
						
						memset(name,0,MAX_NAME_SIZE);
						memset(sub_name,0,MAX_NAME_SIZE);
						logging_state=1;
						state=view;
					}else{
						if(get_after_last_comma(sub_name)!=sub_name)
							memcpy(sub_name+strlen(sub_name)-strlen(get_after_last_comma(sub_name)+1),
									result.requested_str, result.size);
						else
							memcpy(sub_name+strlen(sub_name)-strlen(get_after_last_comma(sub_name)),
									result.requested_str, result.size);
						sub_name[strlen(sub_name)]=',';
						sub_name[strlen(sub_name)]=' ';
						log_selection=-1;
					}
				}
			}

		} else if(state==view){
			if(c =='l'){
				state=logging;
			}else if(c =='s'){
				mvprintw(max_row-3,max_col-sizeof("saved log")+1,"saved log");
				save_log(a_log, database_file);
			}else if(c =='a'){
				state=logging;
				append_log=true;
			}else if(c =='e'){
				mvprintw(max_row-3,max_col-sizeof("ending last entry"),"ending last entry");
				end_last_entry(a_log);
			}else if(c =='q'){
				break;
			}else if(c =='z'){
				if(cell_minutes!=5){
					cell_minutes=cell_minutes-5;
				}
			}else if(c =='x'){
				cell_minutes=cell_minutes+5;
			}else if(c =='c' && entry_under_cursor!=0){
				memcpy(name, entry_under_cursor->name, strlen(entry_under_cursor->name));
				memcpy(sub_name, entry_under_cursor->sub_name, strlen(entry_under_cursor->sub_name));
				change_log=true;
				state=logging;
			}else if(c ==259){
				//uparrow
				cursor_pos_tm-=cell_minutes*60;
			}else if(c ==258){
				//downarrow
				cursor_pos_tm+=cell_minutes*60;
			}else if(c ==339){
				//pgup
				cursor_pos_tm-=cell_minutes*60*4;
			}else if(c ==338){
				//pgdown
				cursor_pos_tm+=cell_minutes*60*4;
			}else if(c ==262){
				//home
				cursor_pos_tm=(unsigned long)time(0);
				cell_minutes=20;
			}
		}

		int y=0,x=0;
		entry_under_cursor=print_logs(a_log,-5,0,max_row,max_col,cell_minutes,cursor_pos_tm);
		//print_logs(a_log,-5,70,max_row,max_col,cell_minutes,cursor_pos_tm-24*60*60);
		//print_logs(a_log,-5,140,max_row,max_col,cell_minutes,cursor_pos_tm-24*60*60*2);
		if(state==logging && logging_state==2){
			result= match_names(max_row-4, 8, a_log, sub_name, log_selection);
		}

		switch (state) {
			case view:{
				mvprintw(max_row-1, 0, "view scale=%d minutes",cell_minutes);
				getyx(stdscr, y, x);
				while(x<max_col-1){
					addch('-');
					getyx(stdscr, y, x);
				}
			}
			break;
			case logging:{
				mvprintw(max_row-1, 0, "logging");
				getyx(stdscr, y, x);
				while(x<max_col-1){
					addch('-');
					getyx(stdscr, y, x);
				}

				mvprintw(max_row-3, 0, "name: %s",name);
				mvprintw(max_row-2, 0, "subname: %s",sub_name);

			}
			break;
		}
		mvprintw(max_row-2,max_col-6,"%d=%d",max_row,max_col);
		mvprintw(max_row-1,max_col-6,"%d=%c",c,c);
		if(UNSAVED_CHANGES){
			const char* msg="unsaved changes";
			attron(COLOR_PAIR(3));
			mvprintw(max_row-1,max_col/2-strlen(msg)/2,"%s",msg);
			attroff(COLOR_PAIR(3));
		}
		refresh();
		c=getch();
		if(c==ERR){
			c=0;
			//ungetch(c);
		}
	}

	endwin();
	return 0;
}
