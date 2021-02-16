#ifndef MAIN_H 
#define MAIN_H

#define MAX_NAME_SIZE 100
#define MAX_STAT_COUNT 70
#define MAX_SAT_CONF_SIZE 500
#define COL_CUTOFF 100
#define AUTOCOM_WIN_MAX_SIZE 20
#define REALLOC_INCREMENT 100
#define ENTRY_TAG_SIZE 20
#define database_file "./cod"
#define net_recieved_database "./cod_net"
#define serv_conf_file "./serv_conf"

enum window_state {
	view,
	week_view,
	stat_view,
	stat_editing,
	stat_add,
	logging,
	append_log, 
	log_editing,
	entry_start_resize,
	entry_end_resize,
	entry_body_resize,
	server_mode,
	delete_mode,
	pause_mode
};

long get_nano_time();

#endif
