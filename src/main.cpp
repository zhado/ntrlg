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
#include "net.cpp"

struct app_state{
	t_log logs;
	char* stat_input;
};

struct server_conf{
	int port;
	int my_port;
	char* ip;
};

bool UNSAVED_CHANGES=false;


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

int load_log(app_state* app,const char* file_name){
	FILE* fp=fopen(file_name,"r");
	if(fp==0){
		fprintf(stderr, "fopen error\n");
		return 1;
	}

	app->logs.allocated=0;
	app->logs.index=0;
	app->logs.entries=0;
	app->stat_input=(char*)calloc(sizeof(char)*MAX_NAME_SIZE,1);

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

			add_entry(&app->logs, temp_name, temp_subname, temp_start_time, temp_end_time);
			memset(temp_name,0,MAX_NAME_SIZE);
			memset(temp_subname,0,MAX_NAME_SIZE);
		}else{
			if(line_index!=0){
				fprintf(stderr, "database file error\n");
				return 1;
			}
			memcpy(app->stat_input, line,strlen(line)-1);
		}
		line_index++;
	}
	if(app->stat_input[0]==0)
		app->logs.index=line_index;
	else
		app->logs.index=line_index-1;

	fclose(fp);
	UNSAVED_CHANGES=false;
	return 0;
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

	free(app->stat_input);
	free(app->logs.entries);
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
			hash+=log_p->entries[i].name[j];
		for(int j=0;j<strlen(log_p->entries[i].sub_name);j++)
			hash+=log_p->entries[i].sub_name[j];
	}
	return hash;
}

int get_log_entry_index(t_log* a_log,log_entry* entry){
	int entry_size=sizeof(log_entry);
	return (entry-a_log->entries);
}

bool crash_with_other_entry(t_log* a_log,log_entry* entry){
	log_entry* next_entry=0;
	log_entry* prev_entry=0;
	time_t local_time=(unsigned long)time(NULL);

	int res_entry_index=get_log_entry_index(a_log, entry);

	if(res_entry_index!=a_log->index-1)
		next_entry=&a_log->entries[res_entry_index+1];
	if(res_entry_index!=0)
		prev_entry=&a_log->entries[res_entry_index-1];

	if(prev_entry ==0 && next_entry==0){
		if(entry->start_time >= entry->end_time){
			return true;
		}
	}else if(prev_entry ==0){
		if(entry->end_time >next_entry->start_time && next_entry!=0){
			return true;
		}else if(entry->start_time >= entry->end_time){
			return true;
		}
	}else if(next_entry==0){
		if(entry->start_time<prev_entry->end_time){
			return true;
		}
	}else{
		if(entry->start_time<prev_entry->end_time){
			return true;
		}else if(entry->end_time >next_entry->start_time && next_entry!=0){
			return true;
		}else if(entry->start_time >= entry->end_time){
			return true;
		}
	}
	if(entry->end_time>local_time || entry->start_time > local_time)
		return	true;

	return false;
}

log_entry* entry_under_cursor_fun(t_log* log_p,int cell_minutes,time_t cursor_pos_tm, int* match_p){
	log_entry* longest_entry=0;

	time_t current_time=(unsigned long)time(0);
	time_t quantized_cursor_pos_tm=cursor_pos_tm-(cursor_pos_tm%(cell_minutes*60));

	time_t curs_start=quantized_cursor_pos_tm;
	time_t curs_end=curs_start+cell_minutes*60;
	time_t last_duration=0;

	//0 nomatch, 1 start match, 2 body match, 3 end match
	int match_type=0;
	for(int i=log_p->index-1;i>=0;i--){
		log_entry* entry=&log_p->entries[i];
		if(entry->end_time>=curs_start && entry->end_time <=curs_end){
			if((entry->end_time-entry->start_time) > last_duration){
				last_duration=entry->end_time-entry->start_time;
				longest_entry=entry;
				match_type=3;
			}
		}else if(entry->end_time==0 && current_time >= curs_start && current_time <=curs_end){
			match_type=3;
			longest_entry=entry;
		}else if(entry->end_time==0 && current_time >= curs_end && entry->start_time <=curs_start){
			match_type=2;
			longest_entry=entry;
		}else if(entry->start_time >= curs_start && entry->start_time <=curs_end && last_duration==0){
			match_type=1;
			longest_entry=entry;
		}else if(entry->start_time<=curs_start && entry->end_time >= curs_end){
			match_type=2;
			longest_entry=entry;
		}
	}
	if(match_p!=0)
		*match_p=match_type;
	return longest_entry;
}

server_conf* load_serv_conf(){
	server_conf* conf=0;
	FILE* fp=fopen(serv_conf_file,"r");
	if(fp==0){
		return conf;
	}
	conf=(server_conf*)calloc(sizeof(server_conf),1);
	conf->ip=(char*)calloc(sizeof(char)*50,1);
	conf->my_port=0;
	char tempchar[100];
	memset(tempchar, 0, 100);
	fgets(tempchar, 100, fp);
	sscanf(tempchar, "port %d",&conf->port);
	fgets(tempchar, 100, fp);
	sscanf(tempchar, "my_port %d",&conf->my_port);
	memset(tempchar, 0, 100);
	fgets(tempchar, 100, fp);
	sscanf(tempchar, "%*s %s",conf->ip);
	return conf;
}

int main(int argc,char** argv){
	int cell_minutes=20;
	time_t cursor_pos_tm=(unsigned long)time(0);
	//t_log* a_log=(t_log*)malloc(sizeof(t_log));
	t_log* a_log;

	server_conf* srv_conf=load_serv_conf();

	int max_row=0,max_col=0;
	window_state state=view;
	
	app_state app;
	app.logs.allocated=0;
	app.logs.index=0;
	app.logs.entries=0;
	app.stat_input=0;
	load_log(&app, database_file);

	char* stat_input=app.stat_input;
	a_log=&app.logs;
	int server_fd=0;
	if(argc==2){
		while(1)
			handle_connections(server_fd);
		exit (1);
	}else{
		//server_fd=setup_server(1902);
	}

	setlocale(LC_CTYPE, "");
	initscr();
	start_color();
	use_default_colors();
	init_pair(1, COLOR_GREEN, -1);
	init_pair(2, -1, COLOR_BLACK);
	init_pair(3, -1, COLOR_MAGENTA);
	init_pair(4, 15, 16);

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

		a_log=&app.logs;
		stat_input=app.stat_input;
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

		} else if(state==entry_start_resize || state==entry_body_resize || state==entry_end_resize){


			time_t initial_cursor_pos_tm=cursor_pos_tm;
			time_t local_time=(unsigned long)time(0);
			time_t initial_start_time=entry_to_resize->start_time;
			time_t initial_end_time=entry_to_resize->end_time;

			if(chr ==259){
				//uparrow
				cursor_pos_tm-=cell_minutes*60;
			}else if(chr ==10){
				state=view;
			}else if(chr ==258){
				//downarrow
				cursor_pos_tm+=cell_minutes*60;
			}else if(chr ==339){
				//pgup
				cursor_pos_tm-=cell_minutes*60*4;
			}else if(chr ==338){
				//pgdown
				cursor_pos_tm+=cell_minutes*60*4;
			}else if(chr =='z'){
				if(cell_minutes!=5){
					cell_minutes=cell_minutes-5;
				}
			}else if(chr =='x'){
				cell_minutes=cell_minutes+5;
			}

			if(state==entry_end_resize){
				entry_to_resize->end_time=cursor_pos_tm;
			}else if(state==entry_start_resize) {
				entry_to_resize->start_time=cursor_pos_tm;
			}else if(state==entry_body_resize) {
				if(entry_to_resize->end_time==0)
					entry_to_resize->end_time=local_time;
				entry_to_resize->start_time+=cursor_pos_tm-initial_cursor_pos_tm;
				entry_to_resize->end_time+=cursor_pos_tm-initial_cursor_pos_tm;
			}

			if(crash_with_other_entry(a_log, entry_to_resize)){
				entry_to_resize->start_time=initial_start_time;
				entry_to_resize->end_time=initial_end_time;
				cursor_pos_tm=initial_cursor_pos_tm;
			}

		} else if(state==server_mode){
			if(chr !=0){
				state=view;
			}else {
				//draw_server_status(0);
				if (handle_connections(server_fd)!=0){
					char msg[]="connectio handling error";
					mvprintw(max_row-4,max_col-sizeof(msg),msg);
				}
				//draw_server_status(1);
			}
		} else if(state==log_editing){

			int res=log_edit(&buffr, chr);
			if(res==0){
				memcpy(entry_under_cursor->name, buffr.name, MAX_NAME_SIZE);
				memcpy(entry_under_cursor->sub_name, buffr.sub_name, MAX_NAME_SIZE);
				state=view;
				entry_under_cursor=0;
			}
		} else if(state==view || state==week_view){

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
				int match_type=0;
				entry_under_cursor=entry_under_cursor_fun(a_log, cell_minutes, cursor_pos_tm,&match_type);
				if(match_type==1){
					entry_to_resize=entry_under_cursor;
					state=entry_start_resize;
				}else if(match_type==2){
					entry_to_resize=entry_under_cursor;
					state=entry_body_resize;
				}else if(match_type==3){
					entry_to_resize=entry_under_cursor;
					state=entry_end_resize;
				}
			}else if(chr =='w'){
				cell_minutes=30;
				cursor_pos_tm=(unsigned long)time(0);
				state=week_view;
			}else if(chr =='v'){
				cursor_pos_tm=(unsigned long)time(0);
				state=view;
			}else if(chr =='e'){
				mvprintw(max_row-3,max_col-sizeof("ending last entry"),"ending last entry");
				end_last_entry(a_log);
			}else if(chr =='H'){
				if(server_fd==0)
					server_fd=setup_server(srv_conf->my_port);
				state=server_mode;
			}else if(chr =='m'){
				draw_error("sandro");
				draw_error("araragi");
			}else if(chr =='L'){
				load_log(&app, database_file);
			}else if(chr =='N'){

				if(srv_conf->ip!=0 && srv_conf->port!=0){
					if(get_from_server(srv_conf->port, srv_conf->ip)==0){
						mvprintw(max_row-3,max_col-sizeof("succsefully recieved dtbs from server"),
								"succsefully recieved dtbs from server");
						free_app(&app);
						load_log(&app, net_recieved_database);
						a_log=&app.logs;
						stat_input=app.stat_input;
						if(remove(net_recieved_database)==0){
							mvprintw(max_row-4,max_col-sizeof("deleted net_recieved_database file"),
									"deleted net_recieved_database file");
						}else{
							mvprintw(max_row-4,max_col-sizeof("error deleteing file"),
									"error deleteing file");
						}
					}else{
						mvprintw(max_row-3,max_col-sizeof("error recieving file"),
								"error recieving file");
					}
				}
			}else if(chr =='q'){
				break;
			}else if(chr =='z'){
				if(cell_minutes!=5){
					cell_minutes=cell_minutes-5;
				}
			}else if(chr =='x'){
				cell_minutes=cell_minutes+5;
			}else if(chr =='c'){
				entry_under_cursor=entry_under_cursor_fun(a_log, cell_minutes, cursor_pos_tm,0);
				if(entry_under_cursor!=0){
					buffr=init_log_edit(a_log, false,entry_under_cursor->name,entry_under_cursor->sub_name);
					state=log_editing;
				}
			}else if(chr ==KEY_LEFT){
				cursor_pos_tm-=60*60*24;
			}else if(chr ==KEY_RIGHT){
				cursor_pos_tm+=60*60*24;
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
		if(state != week_view){
			print_logs(a_log,-5,0,cell_minutes,cursor_pos_tm);
			if(max_col>172)
				draw_durations(23, 90, a_log, stat_input);
		}

		if(state==view){
			//print_logs(a_log,-5,0,max_row,max_col,cell_minutes,cursor_pos_tm);
			curs_set(0);
			mvprintw(max_row-1, 0, "view mode, scale=%d minutes",cell_minutes);
		}else if(state==week_view){
			print_weeks(a_log, cell_minutes, cursor_pos_tm);
			curs_set(0);
			mvprintw(max_row-1, 0, "week view mode, scale=%d minutes",cell_minutes);
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
		}else if(state==entry_start_resize){
			mvprintw(max_row-1, 0, "entry start resize mode");
		}else if(state==server_mode){
			mvprintw(max_row-1, 0, "server_mode");
		}else if(state==entry_body_resize){
			mvprintw(max_row-1, 0, "entry body resize mode");
		}else if(state==entry_end_resize){
			mvprintw(max_row-1, 0, "entry end resize mode");
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

	free(srv_conf->ip);
	free(srv_conf);
	free_app(&app);
	endwin();
	return 0;
}
