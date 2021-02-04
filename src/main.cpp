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
#include "gui_logic.cpp"
#include "logs.cpp"

struct app_state{
	t_log logs;
	char* stat_input;
};

struct server_conf{
	int port;
	int my_port;
	char* ip;
};

struct viewState{
	int cell_minutes=20;
	time_t cursor_pos_tm=(unsigned long)time(0);
	int week_view_width=25;
};
bool UNSAVED_CHANGES=false;

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
			app->stat_input[strlen(line)]=0;
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
	for (int i=0;i<log_p->allocated;i++){
		log_entry* entry=&log_p->entries[i];
		free(entry->name);
		free(entry->sub_name);
	}

	free(app->stat_input);
	free(app->logs.entries);
	app->stat_input=0;
}

uint32_t hash(app_state* app){
	t_log* log_p=&app->logs;
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
	for(int i=0;i<strlen(app->stat_input);i++)
		hash+=app->stat_input[i];
	return hash;
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

long get_nano_time(){
	timespec timee;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&timee);
	return timee.tv_nsec;
}

int main(int argc,char** argv){
	int cell_minutes=20;
	time_t cursor_pos_tm=(unsigned long)time(0);
	int week_view_width=25;

	server_conf* srv_conf=load_serv_conf();

	int max_row=0,max_col=0;
	window_state state=view;
	
	app_state app;
	app.logs.allocated=0;
	app.logs.index=0;
	app.logs.entries=0;
	app.stat_input=0;
	load_log(&app, database_file);

	int server_fd=0;

	if(argc==2){
		server_fd=setup_server(srv_conf->my_port);
		while(1)
			handle_connections(server_fd);
		exit (1);
	}

	setlocale(LC_CTYPE, "");
	initscr();
	nocbreak();
	keypad(stdscr, TRUE);
	noecho();
	raw();
	set_escdelay(20);
	halfdelay(20);

	start_color();
	use_default_colors();
	init_pair(1, COLOR_GREEN, -1);
	init_pair(2, -1, COLOR_BLACK);
	init_pair(3, -1, COLOR_MAGENTA);
	init_pair(4, -1, -1);

	statConfig stat_conf;
	remove_spaces(app.stat_input);
	stat_conf=generate_stat_colors(app.stat_input);
	
	int chr=0;

	log_entry* entry_under_cursor=0;
	log_entry* entry_to_resize=0;
	log_edit_buffer buffr;

	uint32_t last_hash=hash(&app);
	bool are_you_sure_prompt=false;
	int are_you_sure_result=-1;
	bool running=true;
	long start_time,end_time;

	while(running){

		start_time=get_nano_time();
		erase();
		getmaxyx(stdscr,max_row,max_col);

		if (chr == 27){
			mvprintw(max_row-3,max_col-11,"esc pressed");
			entry_to_resize=0;
			state=view;
		}

		if(state==view || state==week_view){

// view keyboard switch --------------------
			switch(chr){
				case 'l':{
					buffr=init_log_edit(&app.logs, false,0,0);
					state=logging;
					chr=0;
				}break;
				case 's':{
					mvprintw(max_row-3,max_col-sizeof("saved log")+1,"saved log");
					save_log(&app, database_file);
				}break;
				case 'a':{
					buffr=init_log_edit(&app.logs, false,0,0);
					state=append_log;
					chr=0;
				}break;
				case 't':{
					buffr=init_log_edit(&app.logs, true,0,app.stat_input);
					stat_conf=generate_stat_colors(app.stat_input);
					chr=0;
					state=stat_editing;
				}break;
				case 'd':{
					int match_type=0;
					entry_under_cursor=entry_under_cursor_fun(&app.logs, cell_minutes, cursor_pos_tm,&match_type);
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
				}break;

				case 'w':{
					cell_minutes=30;
					//cursor_pos_tm=(unsigned long)time(0);
					state=week_view;
				}break;
				case 'v':{
					//cursor_pos_tm=(unsigned long)time(0);
					state=view;
				}break;
				case 'e':{
					mvprintw(max_row-3,max_col-sizeof("ending last entry"),"ending last entry");
					end_last_entry(&app.logs);
				}break;
				case 'H':{
					if(server_fd==0)
						server_fd=setup_server(srv_conf->my_port);
					state=server_mode;
				}break;
				case 'u':{
					free_app(&app);
					load_log(&app,database_file);
					stat_conf=generate_stat_colors(app.stat_input);
				}break;
				case 'q':{
					running=false;
					continue;
				}break;

				case 'N':{
					if(srv_conf->ip!=0 && srv_conf->port!=0){
						if(get_from_server(srv_conf->port, srv_conf->ip)==0){
							mvprintw(max_row-3,max_col-sizeof("succsefully recieved dtbs from server"),
									"succsefully recieved dtbs from server");
							free_app(&app);
							load_log(&app, net_recieved_database);
							stat_conf=generate_stat_colors(app.stat_input);
							if(remove(net_recieved_database)==0){
								mvprintw(max_row-4,max_col-sizeof("deleted net_recieved_database file"),
										"deleted net_recieved_database file");
							}else{
								draw_error("error deleting file");
							}
						}else{
							draw_error("error recieving file");
						}
					}
				}break;

				case 'z':{
					if(cell_minutes!=5){
						cell_minutes=cell_minutes-5;
					}
				}break;
				case 'x':{
					cell_minutes=cell_minutes+5;
				}break;
				case 'Z':{
					week_view_width++;
				}break;
				case 'X':{
					if(week_view_width>0){
						week_view_width--;
					}
				}break;
				case 'D':{
					state=delete_mode;
					are_you_sure_prompt=true;
				}break;
				case 330:{
					state=delete_mode;
					are_you_sure_prompt=true;
				}break;

				case 'c':{
					entry_under_cursor=entry_under_cursor_fun(&app.logs, cell_minutes, cursor_pos_tm,0);
					if(entry_under_cursor!=0){
						buffr=init_log_edit(&app.logs, false,entry_under_cursor->name,entry_under_cursor->sub_name);
						state=log_editing;
					}
				}break;
				case KEY_LEFT:{
					cursor_pos_tm-=60*60*24;
				}break;
				case KEY_RIGHT:{
					cursor_pos_tm+=60*60*24;
				}break;
				case KEY_UP:{
					cursor_pos_tm-=cell_minutes*60;
				}break;
				case KEY_DOWN:{
					cursor_pos_tm+=cell_minutes*60;
				}break;
				case KEY_NPAGE:{
					cursor_pos_tm+=cell_minutes*60*4;
				}break;
				case KEY_PPAGE:{
					cursor_pos_tm-=cell_minutes*60*4;
				}break;
				case KEY_HOME:{
					cursor_pos_tm=(unsigned long)time(0);
					cell_minutes=20;
				}break;
			}
		} else if(state==logging){
			int res=log_edit(&buffr,  chr);
			if(res==0){
				add_entry(&app.logs, buffr.name, buffr.sub_name,(unsigned long)time(0) , 0);
				state=view;
			}
		} else if(state==stat_editing){
			int res=log_edit(&buffr,   chr);
			strcpy(app.stat_input, buffr.sub_name);
			stat_conf=generate_stat_colors(app.stat_input);
			if(res==0){
				state=view;
			}
		} else if(state==append_log){
			int res=log_edit(&buffr, chr);
			if(res==0){
				add_entry(&app.logs, buffr.name, buffr.sub_name,app.logs.entries[app.logs.index-1].end_time , 0);
				state=view;
			}

		} else if(state==entry_start_resize || state==entry_body_resize || state==entry_end_resize){
			resize_logic(&cursor_pos_tm, cell_minutes,  entry_to_resize,&app.logs, chr, &state);

		} else if(state==server_mode){
			if(chr !=0){
				state=view;
			}else {
				if (handle_connections(server_fd)!=0){
					draw_error("connectio handling error");
				}
			}
		} else if(state==delete_mode){
			if(are_you_sure_result==1){
				entry_under_cursor=entry_under_cursor_fun(&app.logs, cell_minutes, cursor_pos_tm,0);
				remove_entry(&app.logs, entry_under_cursor);
			}
			are_you_sure_prompt=false;
			are_you_sure_result=-1;
			state=view;
		} else if(state==log_editing){

			int res=log_edit(&buffr, chr);
			if(res==0){
				memcpy(entry_under_cursor->name, buffr.name, MAX_NAME_SIZE);
				memcpy(entry_under_cursor->sub_name, buffr.sub_name, MAX_NAME_SIZE);
				state=view;
				entry_under_cursor=0;
			}
		}

//drawing happens here --------------------
		print_str_n_times(max_row-1, 0,"-", max_col);
		if(state != week_view){
			print_logs(&app.logs,-5,0,cell_minutes,cursor_pos_tm,&stat_conf);
			if(max_col>172)
				draw_durations(23, 90, &app.logs, &stat_conf);
		}

		if(state==view){
			//print_logs(&app.logs,-5,0,max_row,max_col,cell_minutes,cursor_pos_tm);
			curs_set(0);
			mvprintw(max_row-1, 0, "view mode, scale %d minutes",cell_minutes);
		}else if(state==week_view){
			print_weeks(&app.logs, cell_minutes, cursor_pos_tm,&stat_conf,week_view_width);
			curs_set(0);
			mvprintw(max_row-1, 0, "week view mode, vert_scale %d minutes, hor target scale = %d char",cell_minutes,week_view_width);
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
			dr_text_box(0,0,0,0,"listening for connections");
		}else if(state==entry_body_resize){
			mvprintw(max_row-1, 0, "entry body resize mode");
		}else if(state==entry_end_resize){
			mvprintw(max_row-1, 0, "entry end resize mode");
		}else if(state==delete_mode){
			mvprintw(max_row-1, 0, "delete mode");
			dr_text_box(0,0,0,0,"are you sure you want to delete (y/n)");
		}
		mvprintw(max_row-2,max_col-6,"%d=%d",max_row,max_col);
		mvprintw(max_row-1,max_col-6,"%d=%chr",chr,chr);

		uint32_t new_hash=hash(&app);
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

		mvprintw(max_row-1,max_col-20,"time %f",(get_nano_time()-start_time)/1000000.0);

		move(buffr.cursor_row,buffr.cursor_col);
		chr=getch();
		if(are_you_sure_prompt){
			while(chr != 'y' && chr != 'n' && chr != 'Y' && chr != 'N' ){
				chr=getch();
			}
			are_you_sure_result = (chr == 'y' || chr == 'Y') ? 1 : 0;
			are_you_sure_prompt=false;
		} else if(chr==ERR){
			chr=0;
		}
	}

	free(srv_conf->ip);
	free(srv_conf);
	free_app(&app);
	endwin();
	return 0;
}
