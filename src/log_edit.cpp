#include <ncurses.h>
#include <string.h>
#include "trlg_common.h"
#include "logs.h"
#include "log_edit.h"
#include "trlg_string.h"

log_edit_buffer init_log_edit(t_log* a_log, bool only_tag_str, char* name, char* sub_name){
	log_edit_buffer buffer;
	memset(buffer.name, 0, MAX_NAME_SIZE);
	memset(buffer.sub_name, 0, MAX_NAME_SIZE);
	if(name!=0 && !only_tag_str) strcpy(buffer.name, name);
	if(sub_name!=0)strcpy(buffer.sub_name, sub_name);
	buffer.a_log=a_log;
	buffer.tag_autocomp_selection=-1;
	buffer.only_tag_str=only_tag_str;
	buffer.matched_count=0;
	buffer.cursor_row=0;
	buffer.cursor_col=0;
	return buffer;
}

int log_edit(log_edit_buffer* buffer, int chr){
	int* autocomp_selection=&buffer->tag_autocomp_selection;
	char* name=buffer->name;
	char* tag_str=buffer->sub_name;
	t_log* a_log=buffer->a_log;
	bool only_tag_str=buffer->only_tag_str;

	char* requested_str=buffer->sni[*autocomp_selection].offset;
	int requested_str_size=buffer->sni[*autocomp_selection].size;

	if(chr > 31 && chr <=126){
		if(!only_tag_str){
			if( last_char(name)!=10 && strlen(name) < MAX_NAME_SIZE ){
				name[strlen(name)]=chr;
			}else if ((last_char(name)==10 ) && strlen(tag_str) < MAX_NAME_SIZE){
				tag_str[strlen(tag_str)]=chr;
			}
		}else{
			if (strlen(tag_str) < MAX_NAME_SIZE){
				tag_str[strlen(tag_str)]=chr;
			}
		}
		*autocomp_selection=-1;
	}else if (chr == 263 || chr==127){
		if(!only_tag_str){
			if(last_char(name)!=10 ){
				name[strlen(name)-1]=0;
			}else{
				tag_str[strlen(tag_str)-1]=0;
			}
		}else{
				tag_str[strlen(tag_str)-1]=0;
		}
	}else if (chr == KEY_UP && (last_char(name)==10 || only_tag_str) ){
		if(*autocomp_selection< buffer->matched_count-1)
			*autocomp_selection=*autocomp_selection+1;
	}else if (chr == KEY_DOWN && ( last_char(name)==10 || only_tag_str)){
		if(*autocomp_selection>-1)
			*autocomp_selection=*autocomp_selection-1;
	}else if (chr == 10){
		if(last_char(name)!=10 && !only_tag_str){
			name[strlen(name)]=10;
		}else{
			if(*autocomp_selection==-1){
				if(!only_tag_str)
					name[strlen(name)-1]=0;
				return 0;
			}else{
				if(get_after_last_comma(tag_str)!=tag_str || only_tag_str)
					memcpy(tag_str+strlen(tag_str)-strlen(get_after_last_comma(tag_str)+1),
							requested_str,requested_str_size);
				else
					memcpy(tag_str+strlen(tag_str)-strlen(get_after_last_comma(tag_str)),
							requested_str,requested_str_size);
				tag_str[strlen(tag_str)]=',';
				tag_str[strlen(tag_str)]=' ';
				*autocomp_selection=-1;
			}
		}
	}

	if(last_char(name)==10 || only_tag_str){
		match_names(a_log, tag_str, true,buffer->sni,&buffer->matched_count);
	}
	
	return 1;
}

void draw_log_edit(log_edit_buffer* buffer,int row,int col){
	int* autocomp_selection=&buffer->tag_autocomp_selection;
	char* name=buffer->name;
	char* tag_str=buffer->sub_name;
	t_log* a_log=buffer->a_log;
	bool only_tag_str=buffer->only_tag_str;

	if(last_char(name)==10 || only_tag_str){
		match_names(a_log, tag_str, true,buffer->sni,&buffer->matched_count);
		draw_sni(row-1,col+5,buffer->sni,*autocomp_selection,buffer->matched_count);
	}
	
	if(!only_tag_str){
		mvprintw(row, col, "name: %s",buffer->name);
		mvprintw(row+1, col, "tags: %s",buffer->sub_name);
	}else{
		mvprintw(row, col, "tags: %s",buffer->sub_name);
	}

	if(last_char(name)!=10 && !only_tag_str ){
		buffer->cursor_col=col+ sizeof("name: ")+strlen(buffer->name)-1;
		buffer->cursor_row=row;
	}else{
		buffer->cursor_col=col+ sizeof("tags: ")+strlen(buffer->sub_name)-1;
		if(!only_tag_str)
			buffer->cursor_row=row+1;
		else
			buffer->cursor_row=row;
	}
}

