#include <stdio.h>
#include <time.h>
#include <linux/types.h>
#include <sys/types.h>
#include <ncurses.h>
#include <locale.h>
#include <cstdlib>
#include <math.h>
#include <string.h>

#include "trlg_common.h"
#include "logs.h"
#include "draw.cpp"
#include "log_edit.cpp"
#include "autocomp.cpp"
#include "stats.cpp"
#include "trlg_string.cpp"

struct app_state{
	t_log logs;
	char* stat_input;
};

bool UNSAVED_CHANGES=false;

enum window_state {
	view,
	logging,
	stat_editing,
	append_log, 
	log_editing,
	entry_resize
};

void end_last_entry(t_log* log_p){
	log_entry* entry=&log_p->entries[log_p->index-1];
	if(entry->start_time==0){
		fprintf(stderr, "entry not started.");
	}else if(entry->end_time==0){
		entry->end_time=(unsigned long)time(0);
	}
}

void remove_commas_from_end(char* str){
	while(str[strlen(str)-1]==','){
		str[strlen(str)-1]=0;
	}
}

void add_entry(t_log* log_p, char* name, char* sub_name,time_t start_time,time_t end_time){
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

app_state load_log(const char* file_name){
	FILE* fp=fopen(file_name,"r");
	app_state app;
	app.logs.allocated=0;
	app.logs.index=0;
	app.logs.entries=0;
	app.stat_input=(char*)calloc(sizeof(char)*MAX_NAME_SIZE,1);

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

		if(sscanf(line,"%lu %lu",&temp_start_time,&temp_end_time)==2){
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

			remove_spaces(temp_subname);

			add_entry(&app.logs, temp_name, temp_subname, temp_start_time, temp_end_time);
			memset(temp_name,0,MAX_NAME_SIZE);
			memset(temp_subname,0,MAX_NAME_SIZE);
		}else{
			if(line_index!=0){
				fprintf(stderr, "database file error\n");
				exit(1);
			}
			memcpy(app.stat_input, line,strlen(line)-1);
		}
		line_index++;
	}
	if(app.stat_input[0]==0)
		app.logs.index=line_index;
	else
		app.logs.index=line_index-1;

	fclose(fp);
	UNSAVED_CHANGES=false;
	return app;
}

void save_log(app_state* app, const char* file_name){
	UNSAVED_CHANGES=false;
	t_log* log_p=&app->logs;
	FILE* fp=fopen(file_name,"w");

	fprintf(fp, "%s\n",app->stat_input);
	for(int i=0;i<log_p->index;i++){
		log_entry* entry=&log_p->entries[i];
		fprintf(fp, "%lu %lu \"%s\" \"%s\"\n",entry->start_time,entry->end_time,entry->name,entry->sub_name);
	}

	fclose(fp);
}

void free_app(app_state* app){
	t_log* log_p=&app->logs;
	for (int i=0;i<log_p->index;i++){
		log_entry* entry=&log_p->entries[i];
		free(entry->name);
		free(entry->sub_name);
	}
	free(log_p);
	free(app->stat_input);
	log_p=0;
	app->stat_input=0;
}

uint32_t hash(t_log* log_p){
	uint32_t hash=0;
	for(int i=0;i<log_p->index;i++){
		hash+=log_p->index;
		hash+=log_p->allocated;
		hash+=log_p->entries[i].end_time;
		hash+=log_p->entries[i].start_time;
		for(int j=0;j<strlen(log_p->entries[i].name);j++)
			hash+=*log_p->entries[j].name;
		for(int j=0;j<strlen(log_p->entries[i].sub_name);j++)
			hash+=(int)*log_p->entries[j].sub_name;
	}
	return hash;
}

log_entry* entry_under_cursor_fun(t_log* log_p,int max_row,int cell_minutes,time_t cursor_pos_tm){
	log_entry* longest_entry=0;

	time_t current_time=(unsigned long)time(0);
	time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));

	time_t pos_start=quantized_cursor_pos_tm;
	time_t pos_end=pos_start+cell_minutes*60;
	time_t last_duration=0;
	for(int i=log_p->index-1;i>=0;i--){
		log_entry* entry=&log_p->entries[i];
		if(entry->end_time>=pos_start && entry->end_time <=pos_end){
			if((entry->end_time-entry->start_time) > last_duration){
				last_duration=entry->end_time-entry->start_time;
				longest_entry=entry;
			}
		}else if(entry->end_time==0 && current_time >= pos_start && current_time <=pos_end){
			longest_entry=entry;
		}
		//}else if(entry->start_time >= pos_start && entry->start_time <=pos_end){
			//if((entry->end_time-entry->start_time) > last_duration){
				//last_duration=entry->end_time-entry->start_time;
				//longest_entry=entry;
			//}
		//}
	}
	return longest_entry;
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
	
	app_state app=load_log(database_file);
	//app.stat_input=(char*)malloc(sizeof(char)*MAX_NAME_SIZE);
	char* stat_input=app.stat_input;
	a_log=&app.logs;

	setlocale(LC_CTYPE, "");
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

	int chr=0;

	char name[MAX_NAME_SIZE];
	char sub_name[MAX_NAME_SIZE];
	memset(name,0,MAX_NAME_SIZE);
	memset(sub_name,0,MAX_NAME_SIZE);
	//memset(stat_input,0,MAX_NAME_SIZE);
	//2=subname 1=name
	
	log_entry* entry_under_cursor=0;
	log_entry* entry_to_resize=0;
	log_edit_buffer buffr;
	uint32_t last_hash=hash(a_log);

	//strcpy(sub_name, "x");
	//state=logging;
	while(true){

		erase();
		getmaxyx(stdscr,max_row,max_col);

		if (chr == 27){
			mvprintw(max_row-3,max_col-11,"esc pressed");
			memset(name,0,100);
			memset(sub_name,0,100);
			entry_to_resize=0;
			state=view;
		}

		if(state==logging){
			int res=log_edit(&buffr,  chr);
			if(res==0){
				add_entry(a_log, buffr.name, buffr.sub_name,(unsigned long)time(0) , 0);
				state=view;
			}
		} else if(state==stat_editing){
			int res=log_edit(&buffr,   chr);
			strcpy(stat_input, buffr.sub_name);
			if(res==0){
				state=view;
			}
		} else if(state==append_log){
			int res=log_edit(&buffr, chr);
			if(res==0){
				add_entry(a_log, buffr.name, buffr.sub_name,a_log->entries[a_log->index-1].end_time , 0);
				state=view;
			}

		} else if(state==entry_resize){
			log_entry* entry_ur_cursor_pr=entry_under_cursor_fun(a_log, max_row, cell_minutes, cursor_pos_tm-cell_minutes*60);
			log_entry* entry_ur_cursor_nx=entry_under_cursor_fun(a_log, max_row, cell_minutes, cursor_pos_tm+cell_minutes*60);
			entry_under_cursor=entry_under_cursor_fun(a_log, max_row, cell_minutes, cursor_pos_tm);
			if(chr ==259 && entry_ur_cursor_pr==0){
				//uparrow
				cursor_pos_tm-=cell_minutes*60;
			}else if(chr ==10){
				state=view;
				entry_to_resize=0;
			}else if(chr ==258 && entry_ur_cursor_nx==0){
				//downarrow
				cursor_pos_tm+=cell_minutes*60;
			}else if(chr ==339 && entry_ur_cursor_pr){
				//pgup
				cursor_pos_tm-=cell_minutes*60*4;
			}else if(chr ==338 && entry_ur_cursor_nx){
				//pgdown
				cursor_pos_tm+=cell_minutes*60*4;
			}else if(chr =='z'){
				if(cell_minutes!=5){
					cell_minutes=cell_minutes-5;
				}
			}else if(chr =='x'){
				cell_minutes=cell_minutes+5;
			}
			if(entry_to_resize!=0)
				entry_to_resize->end_time=cursor_pos_tm;

		} else if(state==log_editing){
			int res=log_edit(&buffr, chr);
			if(res==0){
				memcpy(entry_under_cursor->name, buffr.name, MAX_NAME_SIZE);
				memcpy(entry_under_cursor->sub_name, buffr.sub_name, MAX_NAME_SIZE);
				state=view;
				entry_under_cursor=0;
			}
		} else if(state==view){
			if(chr =='l'){
				buffr=init_log_edit(a_log, false,0,0);
				state=logging;
				chr=0;
			}else if(chr =='s'){
				mvprintw(max_row-3,max_col-sizeof("saved log")+1,"saved log");
				save_log(&app, database_file);
			}else if(chr =='a'){
				buffr=init_log_edit(a_log, false,0,0);
				state=append_log;
				chr=0;
			}else if(chr =='t'){
				buffr=init_log_edit(a_log, true,0,stat_input);
				chr=0;
				state=stat_editing;
			}else if(chr =='d'){
				entry_under_cursor=entry_under_cursor_fun(a_log, max_row, cell_minutes, cursor_pos_tm);
				if(entry_under_cursor!=0){
					entry_to_resize=entry_under_cursor;
					state=entry_resize;
				}
			}else if(chr =='e'){
				mvprintw(max_row-3,max_col-sizeof("ending last entry"),"ending last entry");
				end_last_entry(a_log);
			}else if(chr =='q'){
				break;
			}else if(chr =='z'){
				if(cell_minutes!=5){
					cell_minutes=cell_minutes-5;
				}
			}else if(chr =='x'){
				cell_minutes=cell_minutes+5;
			}else if(chr =='c'){
				entry_under_cursor=entry_under_cursor_fun(a_log, max_row, cell_minutes, cursor_pos_tm);
				if(entry_under_cursor!=0){
					buffr=init_log_edit(a_log, false,entry_under_cursor->name,entry_under_cursor->sub_name);
					state=log_editing;
				}
			}else if(chr ==259){
				//uparrow
				cursor_pos_tm-=cell_minutes*60;
			}else if(chr ==258){
				//downarrow
				cursor_pos_tm+=cell_minutes*60;
			}else if(chr ==339){
				//pgup
				cursor_pos_tm-=cell_minutes*60*4;
			}else if(chr ==338){
				//pgdown
				cursor_pos_tm+=cell_minutes*60*4;
			}else if(chr ==262){
				//home
				cursor_pos_tm=(unsigned long)time(0);
				cell_minutes=20;
			}
		}

		//drawing happens here
		print_str_n_times(max_row-1, 0,"-", max_col);
		print_logs(a_log,-5,0,max_row,max_col,cell_minutes,cursor_pos_tm);
		if(max_col>100)
			draw_durations(23, 90, a_log, stat_input);
		if(state==view){
			curs_set(0);
			mvprintw(max_row-1, 0, "view mode, scale=%d minutes",cell_minutes);
		}else if(state==logging){
			curs_set(1);
			draw_log_edit(&buffr, max_row-3, 0);
			mvprintw(max_row-1, 0, "logging");
		}else if(state==stat_editing){
			curs_set(1);
			mvprintw(max_row-1, 0, "stat editing");
			draw_log_edit(&buffr, 20, 90);
		}else if(state==log_editing){
			curs_set(1);
			mvprintw(max_row-1, 0, "log editing");
			draw_log_edit(&buffr, max_row-3, 0);
		}else if(state==append_log){
			curs_set(1);
			draw_log_edit(&buffr, max_row-3, 0);
			mvprintw(max_row-1, 0, "append logging");
		}else if(state==entry_resize){
			curs_set(0);
			mvprintw(max_row-1, 0, "entry resize mode");
		}
		mvprintw(max_row-2,max_col-6,"%d=%d",max_row,max_col);
		mvprintw(max_row-1,max_col-6,"%d=%chr",chr,chr);
		move(buffr.cursor_row,buffr.cursor_col);

		uint32_t new_hash=hash(a_log);
		if(last_hash!=new_hash){
			last_hash=new_hash;
			UNSAVED_CHANGES=true;
		}
		if(UNSAVED_CHANGES){
			const char* msg="unsaved changes";
			attron(COLOR_PAIR(3));
			mvprintw(max_row-1,max_col/2-strlen(msg)/2,"%s",msg);
			attroff(COLOR_PAIR(3));
		}
		refresh();
		chr=getch();
		if(chr==ERR){
			chr=0;
		}
	}

	endwin();
	return 0;
}
