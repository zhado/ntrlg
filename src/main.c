#include <stdio.h>
#include <time.h>
#include <linux/types.h>
#include <ncurses.h>
#include <locale.h>

#include <sys/stat.h>
#include <fcntl.h> 

#include "trlg_common.h"
#include "trlg_string.c"
#include "trlg-unicode.c"
#include "logs.h"
#include "draw.c"
#include "log_edit.c"
#include "autocomp.c"
#include "stats.h"
#include "stats.c"
#include "net.c"
#include "gui_logic.c"
#include "logs.c"

extern NCURSES_EXPORT(int) get_wch (u_int32_t *);

typedef struct {
	t_log logs;
	wchar_t* stat_input;
	statConfig stat_conf;
}app_state;

typedef struct {
	int port;
	int my_port;
	char* ip;
}server_conf;

bool UNSAVED_CHANGES=false;
long get_nano_time(){
	struct timespec Time;
	clock_gettime(CLOCK_REALTIME,&Time);
	return Time.tv_nsec;
}

int parse_line(wchar_t* line,time_t* temp_start_time,time_t* temp_end_time,wchar_t* temp_name,wchar_t* temp_subname){
	int quotes[4]={0,0,0,0},index=0;
	wmemset(temp_name, 0, MAX_NAME_SIZE);
	wmemset(temp_subname, 0, MAX_NAME_SIZE);
	if(swscanf(line,L"%lu %lu",temp_start_time,temp_end_time)==2){
		for(int i=0;i<wcslen(line);i++){
			if(line[i]=='"'){
				quotes[index++]=i;
			}
		}

		// TODO: sahinelebaa es
		if(quotes[1]!=0){
			wmemcpy(temp_name, line+quotes[0]+1,quotes[1]-quotes[0]-1);
		}
		if(quotes[3]!=0){
			wmemcpy(temp_subname, line+quotes[2]+1,quotes[3]-quotes[2]-1);
		}

		remove_spaces(temp_subname);
	}else{
		return 1;
	}
	return 0;
}

int load_log_2(app_state* app,const char* file_name,const char* file_name2){
	bool second_file=true;
	FILE* fp=fopen(file_name,"r");
	FILE* fp2;
	if(fp==0 ){
		fprintf(stderr, "fopen error\n");
		return 1;
	}

	if(file_name2!=0){
		fp2=fopen(file_name2,"r");
		if(fp2==0){
			fprintf(stderr, "fopen error\n");
			return 1;
		}
	}else{
		second_file=false;
	}


	app->logs.allocated=0;
	app->logs.index=0;
	app->logs.entries=0;
	app->stat_input=(wchar_t*)calloc(sizeof(wchar_t)*MAX_NAME_SIZE,1);

	app->logs.tg_alloced=1000;
	app->logs.tg_enrtries=(tgEntry*)calloc(sizeof(tgEntry)*app->logs.tg_alloced,1);
	app->logs.tg_recents=(int*)calloc(sizeof(int)*app->logs.tg_alloced,1);
	app->logs.tg_count=1;

	wchar_t line[MAX_SAT_CONF_SIZE];
	wchar_t line2[MAX_SAT_CONF_SIZE];

	wchar_t stat_conf_bufr[MAX_SAT_CONF_SIZE];
	wmemset(stat_conf_bufr, 0, MAX_SAT_CONF_SIZE);

	wchar_t* fgets_res=(wchar_t*)1;
	wchar_t* fgets_res2=(wchar_t*)1;

	wchar_t temp_name[MAX_NAME_SIZE];
	wchar_t temp_subname[MAX_NAME_SIZE];
	time_t temp_start_time=0;
	time_t temp_end_time=0;
	time_t st_time=0;
	time_t st_time2=0;
	int isconf2;

	// read first lines
	fgets_res=fgetws(line,400,fp);
	int isconf=swscanf(line,L"%lu",&st_time);

	if(isconf!=1 && fgets_res!=0){
		wmemcpy(stat_conf_bufr, line, 500);
		stat_conf_bufr[wcslen(stat_conf_bufr)-1]=0;

		fgets_res=fgetws(line,400,fp);
		isconf=swscanf(line,L"%lu",&st_time);
	}

	if(second_file){
		fgets_res2=fgetws(line2,400,fp2);
		isconf2=swscanf(line2,L"%lu",&st_time2);

		// we ignore second file's statConf
		if(isconf2!=1 && fgets_res2!=0){
			fgets_res2=fgetws(line2,400,fp2);
			isconf2=swscanf(line2,L"%lu",&st_time2);
		}
	}else{
		fgets_res2=0;
	}
	// axla orive pirveli __start_time__ wakiTxuli gvaqvs

	while (1){
		if(fgets_res==0 && fgets_res2==0)
			break;
		if((st_time<st_time2 || fgets_res2==0) && fgets_res!=0){
			parse_line(line,&temp_start_time,&temp_end_time,temp_name,temp_subname);
			fgets_res=fgetws(line,400,fp);
			isconf=swscanf(line,L"%lu",&st_time);
		}else if((st_time>st_time2 || fgets_res==0) && fgets_res2!=0){
			parse_line(line2,&temp_start_time,&temp_end_time,temp_name,temp_subname);
			fgets_res2=fgetws(line2,400,fp2);
			isconf2=swscanf(line2,L"%lu",&st_time2);
		}else if(st_time==st_time2){
			parse_line(line,&temp_start_time,&temp_end_time,temp_name,temp_subname);
			fgets_res=fgetws(line,400,fp);
			fgets_res2=fgetws(line2,400,fp2);
			isconf=swscanf(line,L"%lu",&st_time);
			isconf2=swscanf(line2,L"%lu",&st_time2);
		}
		add_entry(&app->logs, temp_name, temp_subname, temp_start_time, temp_end_time);

		if(app->logs.index>1){
			log_entry* prev_entry=&app->logs.entries[app->logs.index-2];
			log_entry* cur_entry=&app->logs.entries[app->logs.index-1];
			if(prev_entry->end_time > cur_entry->start_time){
				prev_entry->end_time=cur_entry->start_time;
			}
		}
	}

	app->stat_conf=generate_stat_colors(&app->logs,stat_conf_bufr);

	fclose(fp);
	UNSAVED_CHANGES=false;
	return 0;
}

void save_log(app_state* app, const char* file_name){
	UNSAVED_CHANGES=false;
	t_log* log_p=&app->logs;
	FILE* fp=fopen(file_name,"w");

	wchar_t stat_conf_bufr[MAX_SAT_CONF_SIZE]=L"";

	for(int i=0; i < app->stat_conf.count;i++){
		wchar_t* tag_name=get_str_from_id(&app->logs, app->stat_conf.stat_colors[i].tag);
		wcsncat(stat_conf_bufr,tag_name,MAX_NAME_SIZE);
		reconstruct_color(app->stat_conf.stat_colors[i], stat_conf_bufr);
		if(i != app->stat_conf.count-1)
			wcsncat(stat_conf_bufr,L",",MAX_NAME_SIZE);
	}
	fwprintf(fp, L"%ls\n",stat_conf_bufr);

	for(int i=0;i<log_p->index;i++){
		log_entry* entry=&log_p->entries[i];
		wchar_t temp_tag_str[MAX_NAME_SIZE]={0};
		reconstruct_tags(&app->logs, &app->logs.entries[i],temp_tag_str);
		fwprintf(fp, L"%lu %lu \"%ls\" \"%ls\"\n",entry->start_time,entry->end_time,entry->name,temp_tag_str);
	}

	fclose(fp);
}

void free_app(app_state* app){
	t_log* log_p=&app->logs;
	for (int i=0;i<log_p->allocated;i++){
		log_entry* entry=&log_p->entries[i];
		free(entry->name);
		//memset(entry->tags,0,sizeof(int)*ENTRY_TAG_SIZE);
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
		for(int j=0;j<wcslen(log_p->entries[i].name);j++)
			hash+=log_p->entries[i].name[j];
		for(int j=0;;j++){
			if(log_p->entries[i].tags[j]==0)
				break;
			hash+=log_p->entries[i].tags[j];
		}
	}

	for(int i=0;i<app->stat_conf.count;i++){
		hash+=app->stat_conf.stat_colors[i].bg;
		hash+=app->stat_conf.stat_colors[i].fg;
		hash+=app->stat_conf.stat_colors[i].pair_id*i;
		hash+=app->stat_conf.stat_colors[i].tag;
	}
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

time_t get_file_modified_time(char* name){
	struct stat buffer;
	int fd=open(name,O_RDWR);
	fstat(fd, &buffer);
	return buffer.st_mtim.tv_sec;
}

int main(int argc,char** argv){
	int cell_minutes=20;
	time_t cursor_pos_tm=(unsigned long)time(0);
	int week_view_width=25;

	server_conf* srv_conf=load_serv_conf();

	int max_row=0,max_col=0;
	window_state state=view;
	
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

	app_state app;
	app.logs.allocated=0;
	app.logs.index=0;
	app.logs.entries=0;
	app.stat_input=0;
	load_log_2(&app, database_file,0);
	/*load_log_2(&app, database_file,net_recieved_database);*/
	//exit(1);

	int server_fd=0;
	int connection_counter=0;

	if(argc==2){
		server_fd=setup_server(srv_conf->my_port);
		while(1)
			handle_connections(server_fd,&connection_counter);
		exit (1);
	}


	remove_spaces(app.stat_input);
	
	u_int32_t wchr=0;
	int switch_chr=0;

	log_entry* entry_under_cursor=0;
	log_entry* entry_to_resize=0;
	log_edit_buffer buffr;

	uint32_t last_hash=hash(&app);
	time_t last_save_time=0;
	bool week_view_hide_text=false;
	bool are_you_sure_prompt=false;
	bool fude_toggle=true;
	int are_you_sure_result=-1;
	bool running=true;
	struct timespec start_time;
	struct timespec end_time;
	int stat_pos=1;


	while(running){
		clock_gettime(CLOCK_REALTIME,&start_time);

		erase();
		getmaxyx(stdscr,max_row,max_col);

		if (wchr == 27 && state!=pause_mode){
			mvprintw(max_row-3,max_col-11,"esc pressed");
			entry_to_resize=0;
			state=view;
		}
		switch_chr=convert_wide_char(wchr);

		if(state==view || state==week_view){

// view keyboard switch --------------------
			switch(switch_chr){
				case 'l':{
					buffr=init_log_edit(&app.logs, false,0,0);
					state=logging;
				}break;
				case 's':{
					if(last_save_time !=get_file_modified_time(database_file) && last_save_time!=0){
						if (!draw_yn_prompt("file has changed are you sure you want to overwrite? (y/n)"))
							break;
					}

					mvprintw(max_row-3,max_col-sizeof("saved log")+1,"saved log");
					save_log(&app, database_file);
					last_save_time=(unsigned long)time(0);
					last_save_time=get_file_modified_time(database_file);
				}break;
				case 'a':{
					buffr=init_log_edit(&app.logs, false,0,0);
					state=append_log;
				}break;
				case 'd':{
					int entry_part=0;
					entry_under_cursor=entry_under_cursor_fun(&app.logs, cell_minutes, cursor_pos_tm,&entry_part);
					if(entry_part==1){
						entry_to_resize=entry_under_cursor;
						state=entry_start_resize;
					}else if(entry_part==2){
						entry_to_resize=entry_under_cursor;
						state=entry_body_resize;
					}else if(entry_part==3){
						entry_to_resize=entry_under_cursor;
						state=entry_end_resize;
					}
				}break;

				case 'w':{
					state=week_view;
				}break;
				case 'v':{
					state=view;
				}break;
				case 'g':{
					state=stat_view;
				}break;
				case 'e':{
					mvprintw(max_row-3,max_col-sizeof("ending last entry"),"ending last entry");
					end_last_entry(&app.logs);
				}break;
				case 'p':{
					end_last_entry(&app.logs);
					state=pause_mode;
				}break;
				case 'H':{
					if(server_fd==0)
						server_fd=setup_server(srv_conf->my_port);
					state=server_mode;
				}break;
				case 'u':{
					free_app(&app);
					load_log_2(&app,database_file,0);
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
							load_log_2(&app, database_file,net_recieved_database);
							/*if(remove(net_recieved_database)==0){*/
								/*mvprintw(max_row-4,max_col-sizeof("deleted net_recieved_database file"),*/
										/*"deleted net_recieved_database file");*/
							/*}else{*/
								/*draw_error("error deleting file");*/
							/*}*/
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
				case 567:{
					if(cell_minutes!=5){
						cell_minutes=cell_minutes-5;
					}
				}break;
				case 526:{
					cell_minutes=cell_minutes+5;
				}break;
				case 'h':{
					week_view_hide_text=!week_view_hide_text;
				}break;
				case 'f':{
					fude_toggle=!fude_toggle;
				}break;
				case 'Z':{
					week_view_width++;
				}break;
				case 'X':{
					if(week_view_width>1){
						week_view_width--;
					}
				}break;
				case 565:{
					week_view_width++;
				}break;
				case 524:{
					if(week_view_width>1){
						week_view_width--;
					}
				}break;
				case 337:{
					cursor_pos_tm-=60*60*24;
				}break;
				case 336:{
					cursor_pos_tm+=60*60*24;
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
						wchar_t temp_tag_str[MAX_NAME_SIZE]={0};
						reconstruct_tags(&app.logs, entry_under_cursor,temp_tag_str);
						buffr=init_log_edit(&app.logs, false,entry_under_cursor->name,temp_tag_str);
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
				}break;
				case 55:{
					cursor_pos_tm=(unsigned long)time(0);
					week_view_width=25;
					cell_minutes=20;
					week_view_hide_text=false;
				}break;
			}

		} else if(state==stat_view){
			switch(switch_chr){
				case 'v':{
					state=view;
				}break;
				case 'w':{
					state=week_view;
				}break;
				case 's':{
					if(last_save_time !=get_file_modified_time(database_file) && last_save_time!=0){
						if (!draw_yn_prompt("file has changed are you sure you want to overwrite? (y/n)"))
							break;
					}

					mvprintw(max_row-3,max_col-sizeof("saved log")+1,"saved log");
					save_log(&app, database_file);
					last_save_time=(unsigned long)time(0);
					last_save_time=get_file_modified_time(database_file);
				}break;
				case 'c':{
					wchar_t* selected_tag=get_str_from_id(&app.logs, app.stat_conf.stat_colors[app.stat_conf.stat_selection].tag);
					wcscpy(app.stat_input, selected_tag);
					reconstruct_color(app.stat_conf.stat_colors[app.stat_conf.stat_selection], app.stat_input);
					buffr=init_log_edit(&app.logs, true,0,app.stat_input);
					state=stat_editing;
				}break;
				case 'a':{
					memset(app.stat_input, 0, MAX_NAME_SIZE);
					buffr=init_log_edit(&app.logs, true,0,app.stat_input);
					state=stat_add;
				}break;
				case 'd':{
					state=stat_dragging;
				}break;
				case 330:{
					if(app.stat_conf.count>0)
						if(draw_yn_prompt("are you sure you want to delete this tag? (y/n)")==true){
							for(int i=app.stat_conf.stat_selection;i<app.stat_conf.count-1;i++){
								app.stat_conf.stat_colors[i]=app.stat_conf.stat_colors[i+1];
							}
							app.stat_conf.count--;
							if(app.stat_conf.stat_selection>0)
								app.stat_conf.stat_selection--;
						}
				}break;
				case KEY_LEFT:{
					if(stat_pos>1)
						stat_pos=stat_pos-2;
				}break;
				case KEY_RIGHT:{
					stat_pos=stat_pos+2;
				}break;
				case KEY_UP:{
					if(app.stat_conf.stat_selection>0)
						app.stat_conf.stat_selection--;
				}break;
				case KEY_DOWN:{
					if(app.stat_conf.stat_selection<app.stat_conf.count-1)
						app.stat_conf.stat_selection++;
				}break;
				case 'q':{
					running=false;
					continue;
				}break;
			}
		} else if(state==stat_dragging){
			int selection=app.stat_conf.stat_selection;
			statColor temp_statcolor;
			switch(switch_chr){
				case KEY_UP:{
					if(selection!=0){
						temp_statcolor=app.stat_conf.stat_colors[selection];
						app.stat_conf.stat_colors[selection]=app.stat_conf.stat_colors[selection-1];
						app.stat_conf.stat_colors[selection-1]=temp_statcolor;
						app.stat_conf.stat_selection--;
					}
				}break;
				case KEY_DOWN:{
					if(selection!=app.stat_conf.count-1){
						temp_statcolor=app.stat_conf.stat_colors[selection];
						app.stat_conf.stat_colors[selection]=app.stat_conf.stat_colors[selection+1];
						app.stat_conf.stat_colors[selection+1]=temp_statcolor;
						app.stat_conf.stat_selection++;
					}
				}break;
				default:{
					state=stat_view;
				}break;
			}
		} else if(state==logging){
			int res=log_edit(&buffr,&app.logs,  wchr);
			if(res==0){
				add_entry(&app.logs, buffr.name, buffr.tags,(unsigned long)time(0) , 0);
				state=view;
			}
		} else if(state==stat_editing){
			int res=log_edit(&buffr,&app.logs,   wchr);
			wcscpy(app.stat_input, buffr.tags);
			add_statcolor(&app.stat_conf, &app.logs, (strPart){buffr.tags,-1},app.stat_conf.stat_selection);
			if(res==0){
				state=stat_view;
			}
		} else if(state==stat_add){
			int res=log_edit(&buffr,&app.logs,   wchr);
			wcscpy(app.stat_input, buffr.tags);
			if(res==0){
				add_statcolor(&app.stat_conf, &app.logs, (strPart){buffr.tags,-1},-1);
				state=stat_view;
			}
		} else if(state==append_log){
			int res=log_edit(&buffr,&app.logs, wchr);
			if(res==0){
				if(app.logs.entries[app.logs.index-1].end_time==0)
					end_last_entry(&app.logs);
				add_entry(&app.logs, buffr.name, buffr.tags,app.logs.entries[app.logs.index-1].end_time , 0);
				state=view;
			}

		} else if(state==pause_mode){
			switch(switch_chr){
				case 'p':{
					wchar_t* last_entry_name= app.logs.entries[app.logs.index-1].name;
					wchar_t temp_tag_str[MAX_NAME_SIZE]={0};
					reconstruct_tags(&app.logs, &app.logs.entries[app.logs.index-1],temp_tag_str);
					add_entry(&app.logs,last_entry_name,temp_tag_str, (unsigned long)time(0), 0);
					state=view;
				}break;
				default:{
					if(are_you_sure_prompt==false && wchr != 0)
						are_you_sure_prompt=true;
				}break;
				
			}
			if(are_you_sure_result==1){
				state=view;
				are_you_sure_prompt=false;
				are_you_sure_result=-1;
			}else if(are_you_sure_result==0){
				are_you_sure_prompt=false;
				are_you_sure_result=-1;
			}
		} else if(state==entry_start_resize || state==entry_body_resize || state==entry_end_resize){
			resize_logic(&cursor_pos_tm, &cell_minutes,  entry_to_resize,&app.logs, wchr, &state);

		} else if(state==server_mode){
			if(wchr !=0){
				state=view;
			}else {
				if (handle_connections(server_fd,&connection_counter)!=0){
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

			int res=log_edit(&buffr,&app.logs, wchr);
			if(res==0){
				wmemcpy(entry_under_cursor->name, buffr.name, MAX_NAME_SIZE);
				generate_entry_tags(&app.logs, entry_under_cursor, buffr.tags);
				state=view;
				entry_under_cursor=0;
			}
		}

//drawing happens here --------------------
		print_str_n_times(max_row-1, 0,"-", max_col);
		if(state != week_view && state != stat_view && state != stat_editing && state != stat_add && state != stat_dragging){
			print_logs(&app.logs,-5,0,cell_minutes,cursor_pos_tm,&app.stat_conf,state,entry_under_cursor);
			draw_durations(23, 90, &app.logs, &app.stat_conf,stat_pos);
		}

		if(state==view){
			//print_logs(&app.logs,-5,0,max_row,max_col,cell_minutes,cursor_pos_tm);
			curs_set(0);
			mvprintw(max_row-1, 0, "view mode, scale %d minutes",cell_minutes);
		}else if(state==week_view){
			print_weeks(&app.logs, cell_minutes, cursor_pos_tm,&app.stat_conf,week_view_width,week_view_hide_text,fude_toggle);
			curs_set(0);
			mvprintw(max_row-1, 0, "week view mode, vert_scale %d minutes, horz target scale = %d char",cell_minutes,week_view_width);
		}else if(state==stat_view){
			curs_set(0);
			mvprintw(max_row-1, 0, "stats mode stat_selection %d",app.stat_conf.stat_selection);
			draw_durations(2,0, &app.logs, &app.stat_conf,stat_pos);
		}else if(state==stat_dragging){
			curs_set(0);
			mvprintw(max_row-1, 0, "stats dragging stat_selection %d",app.stat_conf.stat_selection);
			draw_durations(2,0, &app.logs, &app.stat_conf,stat_pos);
		}else if(state==pause_mode){
			dr_text_box(0,0,0,0,"pause mode, press p to unpause");
			if(are_you_sure_prompt){
				dr_text_box(0,0,0,0,"are you sure you want to cancel pause mode? (y/n)");
			}
		}else if(state==logging){
			curs_set(1);
			draw_log_edit(&buffr,&app.logs, max_row-3, 0);
			mvprintw(max_row-1, 0, "logging");
		}else if(state==stat_editing){
			curs_set(1);
			mvprintw(max_row-1, 0, "stat editing");
			draw_durations(2,0, &app.logs, &app.stat_conf,stat_pos);
			draw_log_edit(&buffr, &app.logs,20, 0);
		}else if(state==stat_add){
			curs_set(1);
			mvprintw(max_row-1, 0, "stat add");
			draw_durations(2,0, &app.logs, &app.stat_conf,stat_pos);
			draw_log_edit(&buffr, &app.logs,20, 0);
		}else if(state==log_editing){
			curs_set(1);
			mvprintw(max_row-1, 0, "log editing");
			draw_log_edit(&buffr, &app.logs,max_row-3, 0);
		}else if(state==append_log){
			curs_set(1);
			draw_log_edit(&buffr, &app.logs,max_row-3, 0);
			mvprintw(max_row-1, 0, "log append");
		}else if(state==entry_start_resize){
			mvprintw(max_row-1, 0, "entry start resize mode");
		}else if(state==server_mode){
			mvprintw(max_row-1, 0, "server_mode");
			dr_text_box(0,0,0,0,"listening for connections");
			mvprintw(max_row/2-1, max_col/2-12, "handled connections %d",connection_counter);
		}else if(state==entry_body_resize){
			mvprintw(max_row-1, 0, "entry body resize mode");
		}else if(state==entry_end_resize){
			mvprintw(max_row-1, 0, "entry end resize mode");
		}else if(state==delete_mode){
			mvprintw(max_row-1, 0, "delete mode");
			dr_text_box(0,0,0,0,"are you sure you want to delete (y/n)");
		}
		mvprintw(max_row-2,max_col-6,"%d=%d",max_row,max_col);
		mvprintw(max_row-1,max_col-6,"%ld=%lc",wchr,wchr);

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

		clock_gettime(CLOCK_REALTIME,&end_time);
		mvprintw(max_row-1,max_col-20,"time %f",(end_time.tv_nsec-start_time.tv_nsec)/1000000.0);

		move(buffr.cursor_row,buffr.cursor_col);

#ifdef ANDROID	
		wchr=getch();
#else
		get_wch(&wchr);
#endif

		if(are_you_sure_prompt){
			while(wchr != 'y' && wchr != 'n' && wchr != 'Y' && wchr != 'N' ){
				wchr=getch();
			}
			are_you_sure_result = (wchr == 'y' || wchr == 'Y') ? 1 : 0;
		} else if(wchr==ERR){
			wchr=0;
			
		}
	}

	free(srv_conf->ip);
	free(srv_conf);
	free_app(&app);
	endwin();
	return 0;
}

