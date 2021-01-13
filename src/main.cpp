#include <stdio.h>
#include <time.h>
#include <linux/types.h>
#include <sys/types.h>
#include <ncurses.h>
#include <cstdlib>
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

struct log {
	int index;
	int allocated;
	log_entry* entries;
};

void print_normal_time(time_t tim){
	tm* broken_down_time=localtime(&tim);
	printw("%d/%d/%d %02d:%02d:%02d",
			broken_down_time->tm_mday,
			broken_down_time->tm_mon+1,
			broken_down_time->tm_year+1900,
			broken_down_time->tm_hour,
			broken_down_time->tm_min,
			broken_down_time->tm_sec); 
}

void end_last_entry(log* log_p){
	log_entry* entry=&log_p->entries[log_p->index-1];
	if(entry->start_time==0){
		fprintf(stderr, "entry not started.");
	}else if(entry->end_time==0){
		entry->end_time=(unsigned long)time(NULL);
	}
}

void start_entry(log* log_p, char* name, char* sub_name){
	if(log_p->index!=0)
		end_last_entry(log_p);
	//printf("%d < %d == %d\n",log_p->allocated , (log_p->index+1),log_p->allocated < (log_p->index+1));
	// aq raxdeba?????????
	if(log_p->allocated < (log_p->index+1) ){
		printf("realocing...\n");
		log_p->entries=(log_entry*)realloc(log_p->entries, sizeof(log_entry)*(log_p->allocated+realloc_increment));
		log_p->allocated=log_p->allocated+realloc_increment;
	}
	log_entry* entry=&log_p->entries[log_p->index];

	entry->name=(char*)malloc(sizeof(char)*max_name_size);
	entry->sub_name=(char*)malloc(sizeof(char)*max_name_size);
	entry->end_time=0;

	entry->start_time=(unsigned long)time(NULL);
	strcpy(entry->name, name);
	strcpy(entry->sub_name, sub_name);
	log_p->index++;
}


void print_logs(log* log_p){
	move(0,0);
	for(int i=0;i<log_p->index;i++){
		log_entry* entry=&log_p->entries[i];
		print_normal_time(entry->start_time);
		printw(" - ");
		if(entry->end_time == 0) 
			printw("now");
		else 
			print_normal_time(entry->end_time);
		printw(" %s, %s\n",entry->name,entry->sub_name);
	}
}

log* load_log(char* file_name){
	FILE* fp=fopen(file_name,"r");
	log* a_log=(log*)malloc(sizeof(log));
	a_log->entries=(log_entry*)malloc(sizeof(log_entry)*100);
	a_log->allocated=100;
	char line[200];
	int line_index=0;
	while (fgets(line,200,fp)!=0){
		int quotes[4]={0,0,0,0},index=0;
		for(int i=0;i<strlen(line);i++){
			if(line[i]=='"'){
				quotes[index++]=i;
			}
		}

		a_log->entries[line_index].name=(char*)calloc(sizeof(char)*max_name_size,1);
		a_log->entries[line_index].sub_name=(char*)calloc(sizeof(char)*max_name_size,1);

		// TODO: sahinelebaa es
		if(quotes[1]!=0){
			memcpy(a_log->entries[line_index].name, line+quotes[0]+1,quotes[1]-quotes[0]-1);
		}

		if(quotes[3]!=0){
			memcpy(a_log->entries[line_index].sub_name, line+quotes[2]+1,quotes[3]-quotes[2]-1);
		}

		sscanf(line,"%lu %lu",&a_log->entries[line_index].start_time,&a_log->entries[line_index].end_time);

		line_index++;
		//TODO: es shesamowmebelia
		if(line_index>a_log->allocated){
			printf("reallocing during import\n");
			a_log->entries=(log_entry*)realloc(a_log->entries,sizeof(log_entry)*( a_log->allocated+realloc_increment));
			a_log->allocated=a_log->allocated+realloc_increment;
		}
	}
	a_log->index=line_index;

	fclose(fp);
	return a_log;
}

void save_log(log* log_p, char* file_name){
	FILE* fp=fopen(file_name,"w");

	for(int i=0;i<log_p->index;i++){
		log_entry* entry=&log_p->entries[i];
		fprintf(fp, "%lu %lu \"%s\" \"%s\"\n",entry->start_time,entry->end_time,entry->name,entry->sub_name);
	}

	fclose(fp);
}

void free_log(log* log_p){
	for (int i=0;i<log_p->index;i++){
		log_entry* entry=&log_p->entries[i];
		free(entry->name);
		free(entry->sub_name);
	}
	free(log_p);
}

int main(){
	time_t epoch_time=(unsigned long)time(NULL);
	log* a_log=(log*)malloc(sizeof(log));

	a_log->index=0;
	a_log->entries=(log_entry*)malloc(sizeof(log_entry)*100);
	a_log->allocated=100;
	char* input=(char*)malloc(100);
	int max_row=0,max_col=0;
	window_state state=view;

	initscr();
	cbreak();
	//raw();
	//keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	char c=0;
	free_log(a_log);
	a_log=load_log("cod");
	char command[100];

	char name[100];
	char subname[100];
	// 1=name
	// 2=subname
	int logging_state=1;

	memset(input,0,100);
	memset(command,0,100);
	while(true){

		erase();
		getmaxyx(stdscr,max_row,max_col);
		if(state==logging){
			if(c > 31 && c <=126){
				if(logging_state==1){
					name[strlen(name)]=c;
				}else{
					subname[strlen(subname)]=c;
				}
			}else if (c == 127){
				if(logging_state==1){
					name[strlen(name)-1]=0;
				}else{
					subname[strlen(subname)-1]=0;
				}
			}else if (c == 10){
				//handle_command(a_log, input, command);
				if(logging_state==1){
					logging_state++;
				}else{
					start_entry(a_log, name, subname);
					memset(name,0,100);
					memset(subname,0,100);
					logging_state=1;
					state=view;
				}
			}
		}
		if (c == 27){
			mvprintw(max_row-2,max_col-11,"esc pressed");
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
			}else if(c =='e'){
				mvprintw(max_row-2,max_col-sizeof("ending last entry"),"ending last entry");
				end_last_entry(a_log);
			}else if(c =='q'){
				break;
			}
		}
		int y=0,x=0;
		print_logs(a_log);

		switch (state) {
			case view:{
				curs_set(0);
				mvprintw(max_row-1, 0, "view");
				getyx(stdscr, y, x);
				while(x<max_col-1){
					addch('-');
					getyx(stdscr, y, x);
				}
			}
			break;
			case logging:{
				curs_set(1);
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
		mvprintw(max_row-1,max_col-6,"%d=%c",c,c);
		refresh();
		c=getch();
	}

	endwin();
	return 0;
}
