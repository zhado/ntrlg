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

void print_normal_time(time_t tim){
	tm* broken_down_time=localtime(&tim);
	printw("%02d:%02d",
			broken_down_time->tm_hour,
			broken_down_time->tm_min); 
}

void end_last_entry(t_log* log_p){
	log_entry* entry=&log_p->entries[log_p->index-1];
	if(entry->start_time==0){
		fprintf(stderr, "entry not started.");
	}else if(entry->end_time==0){
		entry->end_time=(unsigned long)time(NULL);
	}
}

void start_entry(t_log* log_p, char* name, char* sub_name){
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


void draw_time_boxes(t_log* logp,time_t cell_tm, int cell_minutes,int cur_row){
	for(int i=logp->index-1;i>=0;i--){
		time_t start_tm=logp->entries[i].start_time;
		time_t end_time=logp->entries[i].end_time;
		time_t delta=end_time-start_tm;
		if((end_time<cell_tm+(cell_minutes*60)) && end_time> cell_tm){
			mvprintw(cur_row, 20, "+----+");
			int j=0;
			for(;j<delta/(cell_minutes*60);j++){
				if(cur_row-j>0)
					mvprintw(cur_row-j, 20, "|----|");
				else  
					break;
			}
			mvprintw(cur_row-j, 20, "*----*");
		}
	}
}

void print_logs(t_log* log_p,int max_row,int max_col,int cell_offset,int cell_minutes){
	int count=0;
	int last_remender=0;

	time_t epoch_tm=(unsigned long)time(NULL);
	tm* broken_down_time=localtime(&epoch_tm);
	time_t nexthour_timestamp=(unsigned long)time(NULL)+(cell_minutes*60-(broken_down_time->tm_min%cell_minutes)*60-broken_down_time->tm_sec)-cell_minutes*60*cell_offset;

	for(int i=max_row-5;i>=0;i--){
		time_t cell_tm=nexthour_timestamp-cell_minutes*60*count;
		tm* broken_down_cell_tm=localtime(&cell_tm);

		move(i,0);
		//printw("----------- ");

		//print_normal_time(cell_tm);

		mvprintw(i,0,"%02d:%02d",broken_down_cell_tm->tm_hour,broken_down_cell_tm->tm_min); 
		if(broken_down_cell_tm->tm_min==0){
			mvprintw(i,6," %02d",broken_down_cell_tm->tm_hour);
			if(broken_down_cell_tm->tm_hour==0)
				mvprintw(i,10," %02d/%02d/%02d",broken_down_cell_tm->tm_mday,broken_down_cell_tm->tm_mon+1,broken_down_cell_tm->tm_year+1900);
		}
		//printw("from %d to %d ",cell_tm,cell_tm+cell_minutes*60);
		draw_time_boxes(log_p,cell_tm,cell_minutes,i);

		count++;
	}
	//for(int i=0;i<log_p->index;i++){
		//log_entry* entry=&log_p->entries[i];
		//print_normal_time(entry->start_time);
		//printw(" - ");
		//if(entry->end_time == 0) 
			//printw("now");
		//else 
			//print_normal_time(entry->end_time);
		//printw(" %s, %s\n",entry->name,entry->sub_name);
	//}
}

t_log* load_log(char* file_name){
	FILE* fp=fopen(file_name,"r");
	t_log* a_log=(t_log*)malloc(sizeof(t_log));
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
	int cell_offset=0,cell_minutes=20;
	time_t epoch_time=(unsigned long)time(NULL);
	t_log* a_log=(t_log*)malloc(sizeof(t_log));

	a_log->index=0;
	a_log->entries=(log_entry*)malloc(sizeof(log_entry)*100);
	a_log->allocated=100;
	char* input=(char*)malloc(100);
	int max_row=0,max_col=0;
	window_state state=view;

	initscr();
	cbreak();
	raw();
	set_escdelay(4);
	keypad(stdscr, TRUE);
	//ctrl('k');
	//halfdelay(1);
	noecho();
	//timeout(0);
	curs_set(0);

	int c=0;
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
			}else if (c == 263){
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
			}else if(c =='e'){
				mvprintw(max_row-2,max_col-sizeof("ending last entry"),"ending last entry");
				end_last_entry(a_log);
			}else if(c =='q'){
				break;
			}else if(c ==259){
				cell_offset++;
			}else if(c ==258){
				if(cell_offset>0)
					cell_offset--;
			}
		}
		int y=0,x=0;
		print_logs(a_log,max_row,max_col,cell_offset,cell_minutes);

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
		mvprintw(max_row-2,max_col-6,"%d=%d",max_row,max_col);
		mvprintw(max_row-1,max_col-6,"%d=%c",c,c);
		refresh();
		c=getch();
	}

	endwin();
	return 0;
}
