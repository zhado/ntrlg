#include <stdio.h>
#include <time.h>
#include <linux/types.h>
#include <sys/types.h>
#include <ncurses.h>
#include <cstdlib>
#include <math.h>
#include <string.h>
typedef unsigned long ul64;

const int max_name_size=100;
const int realloc_increment=100;

enum window_state {
	view,logging
};
struct log_entry {
	char* name;
	char* sub_name;
	time_t start_time;
	time_t end_time;
};

struct t_log {
	int index;
	int allocated;
	log_entry* entries;
};

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

void end_last_entry(t_log* log_p){
	log_entry* entry=&log_p->entries[log_p->index-1];
	if(entry->start_time==0){
		fprintf(stderr, "entry not started.");
	}else if(entry->end_time==0){
		entry->end_time=(unsigned long)time(0);
	}
}

void append_entry(t_log* log_p, char* name, char* sub_name,time_t start_time,time_t end_time){
	if(log_p->index!=0)
		end_last_entry(log_p);
	if(log_p->allocated < (log_p->index+1) ){
		printf("realocing...\n");
		log_p->entries=(log_entry*)realloc(log_p->entries, sizeof(log_entry)*(log_p->allocated+realloc_increment));
		log_p->allocated=log_p->allocated+realloc_increment;
	}
	log_entry* entry=&log_p->entries[log_p->index];

	entry->name=(char*)calloc(sizeof(char)*max_name_size,1);
	entry->sub_name=(char*)calloc(sizeof(char)*max_name_size,1);
	entry->end_time=end_time;

	entry->start_time=start_time;
	strcpy(entry->name, name);
	strcpy(entry->sub_name, sub_name);
	log_p->index++;
}

void draw_time_boxes(t_log* logp,time_t cell_tm, int cell_minutes,int cur_row){
	time_t next_cell_tm=cell_tm+(cell_minutes*60);
	time_t local_time=(unsigned long)time(0);
	if(cell_tm<local_time && next_cell_tm > local_time){
			mvprintw(cur_row, 26, "<-- now");
	}
	for(int i=logp->index-1;i>=0;i--){
		time_t start_tm=logp->entries[i].start_time;
		time_t end_time=logp->entries[i].end_time;
		if(end_time < next_cell_tm && end_time > cell_tm){
			log_entry* entry=&logp->entries[i];
			mvprintw(cur_row, 20, "=---->");
			printw("%s ",entry->name);
			print_duration(entry->end_time-entry->start_time);
			break;
		}else if(end_time==0 && cell_tm>start_tm && next_cell_tm > local_time && cell_tm < local_time ){
			log_entry* entry=&logp->entries[logp->index-1];
			mvprintw(cur_row, 20, "++++++");
			printw("%s ",entry->name);
			print_duration(local_time-entry->start_time);
			break;
		}else if(((next_cell_tm<end_time || end_time==0 )&& cell_tm < local_time) && cell_tm>start_tm){
			mvprintw(cur_row, 20, "|    |");
			break;
		}
	}
}

time_t round_tm(time_t timestamp, int cell_minutes){
	tm* broken_down_time=localtime(&timestamp);
	time_t nexthour_timestamp=timestamp+(cell_minutes*60-(broken_down_time->tm_min%cell_minutes)*60-broken_down_time->tm_sec);
	return nexthour_timestamp;
}
void print_logs(t_log* log_p,int max_row,int max_col,int cell_minutes,time_t cursor_pos_tm){
	int count=0;

	tm* broken_down_time=localtime(&cursor_pos_tm);
	time_t nexthour_timestamp=cursor_pos_tm+(cell_minutes*60-(broken_down_time->tm_min%cell_minutes)*60-broken_down_time->tm_sec);
	nexthour_timestamp=nexthour_timestamp-cell_minutes*60;

	for(int i=max_row-5;i>=0;i--){
		time_t cell_tm=nexthour_timestamp-cell_minutes*60*count;
		tm* broken_down_cell_tm=localtime(&cell_tm);

		move(i,0);
		mvprintw(i,0,"%02d:%02d",broken_down_cell_tm->tm_hour,broken_down_cell_tm->tm_min); 
		if(broken_down_cell_tm->tm_min==0){
			mvprintw(i,6," %02d",broken_down_cell_tm->tm_hour);
			if(broken_down_cell_tm->tm_hour==0)
				mvprintw(i,10," %02d/%02d/%02d",broken_down_cell_tm->tm_mday,broken_down_cell_tm->tm_mon+1,broken_down_cell_tm->tm_year+1900);
		}
		draw_time_boxes(log_p,cell_tm,cell_minutes,i);

		count++;
	}

	if(max_col>90)
	for(int i=0;i<log_p->index;i++){
		log_entry* entry=&log_p->entries[i];
		print_normal_time(0+i,60,entry->start_time);
		if(entry->end_time == 0) 
			mvprintw(0+i,69,"now");
		else 
			print_normal_time(0+i,67,entry->end_time);
		printw(" %s, %s\n",entry->name,entry->sub_name);
	}
}

t_log* load_log(char* file_name){
	FILE* fp=fopen(file_name,"r");
	t_log* a_log=(t_log*)malloc(sizeof(t_log));
	a_log->allocated=0;
	a_log->index=0;

	char line[400];
	int line_index=0;
	while (fgets(line,400,fp)!=0){
		int quotes[4]={0,0,0,0},index=0;
		char temp_name[max_name_size];
		char temp_subname[max_name_size];
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

		append_entry(a_log, temp_name, temp_subname, temp_start_time, temp_end_time);
		line_index++;
		memset(temp_name,0,max_name_size);
		memset(temp_subname,0,max_name_size);
	}
	a_log->index=line_index;

	fclose(fp);
	return a_log;
}

void save_log(t_log* log_p, char* file_name){
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

	initscr();
	cbreak();
	raw();
	set_escdelay(20);
	keypad(stdscr, TRUE);
	//ctrl('k');
	//halfdelay(1);
	noecho();
	//timeout(0);
	curs_set(0);

	int c=0;
	free_log(a_log);
	a_log=load_log("cod");

	char name[max_name_size];
	char subname[max_name_size];
	// 1=name
	// 2=subname
	int logging_state=1;
	bool append_log=false;

	while(true){

		erase();
		getmaxyx(stdscr,max_row,max_col);
		if(state==logging){
			if(c > 31 && c <=126){
				if(logging_state==1 && strlen(name) < max_name_size){
					name[strlen(name)]=c;
				}else if ( logging_state ==2 && strlen(subname) < max_name_size){
					subname[strlen(subname)]=c;
				}
			}else if (c == 263){
				if(logging_state==1){
					name[strlen(name)-1]=0;
				}else{
					subname[strlen(subname)-1]=0;
				}
			}else if (c == 10){
				if(logging_state==1){
					logging_state++;
				}else{
					if(append_log){
						append_entry(a_log, name, subname,a_log->entries[a_log->index-1].end_time,0);
						append_log=false;
					}else{
						append_entry(a_log, name, subname,(unsigned long)time(0),0);
					}
					memset(name,0,max_name_size);
					memset(subname,0,max_name_size);
					logging_state=1;
					state=view;
				}
			}
		}
		if (c == 27){
			mvprintw(max_row-3,max_col-11,"esc pressed");
			memset(name,0,100);
			memset(subname,0,100);
			state=view;
			logging_state=1;
		}

		if(state==view){
			if(c =='l'){
				state=logging;
			}else if(c =='s'){
				mvprintw(max_row-2,max_col-sizeof("saved log"),"saved log");
				save_log(a_log, "cod");
			}else if(c =='a'){
				state=logging;
				append_log=true;
			}else if(c =='e'){
				mvprintw(max_row-2,max_col-sizeof("ending last entry"),"ending last entry");
				end_last_entry(a_log);
			}else if(c =='q'){
				break;
			}else if(c =='z'){
				if(cell_minutes!=5){
					cell_minutes=cell_minutes-5;
				}
			}else if(c =='x'){
				cell_minutes=cell_minutes+5;
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
		mvprintw(max_row/2-5,0,"_________________________________________");
		print_logs(a_log,max_row,max_col,cell_minutes,cursor_pos_tm+cell_minutes*max_row/2*60);

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
				mvprintw(max_row-2, 0, "subname: %s",subname);

			}
			break;
		}
		mvprintw(max_row-2,max_col-6,"%d=%d",max_row,max_col);
		mvprintw(max_row-1,max_col-6,"%d=%c",c,c);
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
