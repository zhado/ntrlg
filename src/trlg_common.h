#ifndef MAIN_H 
#define MAIN_H

#define MAX_NAME_SIZE 100
#define COL_CUTOFF 100
#define AUTOCOM_WIN_MAX_SIZE 20
#define REALLOC_INCREMENT 100
#define database_file "./cod"
#define net_recieved_database "./cod_net"
#define serv_conf_file "./serv_conf"

enum window_state {
	view,
	week_view,
	logging,
	stat_editing,
	append_log, 
	log_editing,
	entry_start_resize,
	entry_end_resize,
	entry_body_resize,
	server_mode,
	delete_mode
};

#endif
