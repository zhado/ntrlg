#ifndef MAIN_H 
#define MAIN_H

#define MAX_NAME_SIZE 100
#define MAX_STAT_COUNT 70
#define MAX_SAT_CONF_SIZE 500
#define MAX_TAG_COUNT 20
#define COL_CUTOFF 100
#define AUTOCOM_WIN_MAX_SIZE 20
#define REALLOC_INCREMENT 100
#define ENTRY_TAG_SIZE 20
#define database_file "/home/zado/code/trlg/cod"
#define net_recieved_database "/home/zado/code/trlg/cod_net"
#define serv_conf_file "/home/zado/code/trlg/serv_conf"

typedef enum {
	view,
	week_view,
	stat_view,
	stat_editing,
	stat_dragging,
	stat_add,
	logging,
	append_log, 
	log_editing,
	log_insert,
	entry_start_resize,
	entry_end_resize,
	entry_body_resize,
	server_mode,
	delete_mode,
	pause_mode
}window_state ;

long get_nano_time();

#endif
