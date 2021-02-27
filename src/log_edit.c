#include <ncurses.h>
#include <string.h>

#include "trlg_common.h"
#include "logs.h"
#include "log_edit.h"
#include "trlg_string.h"

log_edit_buffer init_log_edit(t_log* a_log, bool only_tag_str, wchar_t* name, wchar_t* sub_name){
	log_edit_buffer buffer;
	memset(buffer.name, 0, MAX_NAME_SIZE);
	memset(buffer.sub_name, 0, MAX_NAME_SIZE);
	if(name!=0 && !only_tag_str) wcscpy(buffer.name, name);
	if(sub_name!=0)wcscpy(buffer.sub_name, sub_name);
	buffer.a_log=a_log;
	buffer.tag_autocomp_selection=-1;
	buffer.only_tag_str=only_tag_str;
	buffer.matched_count=0;
	buffer.cursor_row=0;
	buffer.cursor_col=0;
	if(!only_tag_str){
		buffer.local_curs_pos=wcslen(buffer.name);
	}else{
		buffer.local_curs_pos=wcslen(buffer.sub_name);
	}

	return buffer;
}

int log_edit(log_edit_buffer* buffer,t_log* log_p, wchar_t chr){
	int* autocomp_selection=&buffer->tag_autocomp_selection;
	wchar_t* name=buffer->name;
	wchar_t* tag_str=buffer->sub_name;
	t_log* a_log=buffer->a_log;
	bool only_tag_str=buffer->only_tag_str;

	int selected_id=0;
	int requested_str_size=0;
	char* requested_str=0;


	if(*autocomp_selection!=-1){
		selected_id=buffer->sni[*autocomp_selection].tag_id;
		requested_str=log_p->tg_enrtries[selected_id].tag;
		requested_str_size=strlen(log_p->tg_enrtries[selected_id].tag);
	}
	bool editing_tags=false;
	if(buffer->name[wcslen(buffer->name)-1]==10){
		editing_tags=true;
	}

	if(chr > 31 && chr !=260 && chr !=261 && chr !=258 && chr != 259 &&chr != 127){
		if( !editing_tags && wcslen(name) < MAX_NAME_SIZE -2 && !only_tag_str){
			add_chr_in_wstr(chr, name, buffer->local_curs_pos, MAX_NAME_SIZE);
			buffer->local_curs_pos++;
		}else if (wcslen(tag_str) < MAX_NAME_SIZE-2){
			add_chr_in_wstr(chr, tag_str, buffer->local_curs_pos, MAX_NAME_SIZE);
			buffer->local_curs_pos++;
		}
		*autocomp_selection=-1;
	}else if (chr == 263 || chr==127){
		if(!editing_tags && !only_tag_str){
			int name_len=wcslen(name);
			if(name_len>0 && buffer->local_curs_pos>0){
				for(int i=buffer->local_curs_pos-1;i<name_len;i++){
					buffer->name[i]=buffer->name[i+1];
				}
				buffer->local_curs_pos--;
			}
		}else{
			int tag_len=wcslen(tag_str);
			if(tag_len>0 && buffer->local_curs_pos>0){
				for(int i=buffer->local_curs_pos-1;i<tag_len;i++){
					buffer->sub_name[i]=buffer->sub_name[i+1];
				}
				buffer->local_curs_pos--;
			}
		}
	}else if (chr == KEY_UP && (!editing_tags || only_tag_str) ){
		if(*autocomp_selection< buffer->matched_count-1)
			*autocomp_selection=*autocomp_selection+1;
	}else if (chr == KEY_DOWN && ( !editing_tags || only_tag_str)){
		if(*autocomp_selection>-1)
			*autocomp_selection=*autocomp_selection-1;
	}else if (chr == KEY_RIGHT ){
		if( (!editing_tags || only_tag_str) &&( wcslen(buffer->sub_name) >buffer->local_curs_pos)){
			buffer->local_curs_pos++;
		}else if(last_wchar(name)!=10 && wcslen(buffer->name)>buffer->local_curs_pos){
			buffer->local_curs_pos++;
		}
	}else if (chr == KEY_LEFT && buffer->local_curs_pos>0){
			buffer->local_curs_pos--;
	}else if (chr == 10){
		int tag_str_len=wcslen(tag_str);
		if(last_wchar(name)!=10 && !only_tag_str){
			name[wcslen(name)]=10;
			buffer->local_curs_pos=wcslen(buffer->sub_name);
		}else{
			if(*autocomp_selection==-1){
				if(!only_tag_str)
					name[wcslen(name)-1]=0;
				return 0;
			}else{
				if(get_after_last_comma(tag_str)!=tag_str || only_tag_str)
					memcpy(tag_str+tag_str_len-strlen(get_after_last_comma(tag_str))+1*(tag_str[strlen(tag_str)-1]==32),
							requested_str,requested_str_size);
				else
					memcpy(tag_str+strlen(tag_str)-strlen(get_after_last_comma(tag_str)),
							requested_str,requested_str_size);
				tag_str[strlen(tag_str)]=',';
				tag_str[strlen(tag_str)]=' ';
				buffer->local_curs_pos=strlen(tag_str);
				*autocomp_selection=-1;
			
			}
		}
	}

	if(!editing_tags || only_tag_str){
		match_names(a_log, tag_str,buffer->sni,&buffer->matched_count);
	}
	
	return 1;
}

void draw_log_edit(log_edit_buffer* buffer,t_log* log_p,int row,int col){
	int* autocomp_selection=&buffer->tag_autocomp_selection;
	wchar_t* name=buffer->name;
	wchar_t* tag_str=buffer->sub_name;
	t_log* a_log=buffer->a_log;
	bool only_tag_str=buffer->only_tag_str;
	
	bool editing_tags=false;
	if(buffer->name[wcslen(buffer->name)-1]==10){
		editing_tags=true;
	}

	if(editing_tags || only_tag_str){
		match_names(a_log, tag_str,buffer->sni,&buffer->matched_count);
		draw_autocomp(row-1,col+5,log_p,buffer->sni,*autocomp_selection,buffer->matched_count);
	}
	
	if(!only_tag_str){
		mvprintw(row, col, "name: %ls",buffer->name);
		mvprintw(row+1, col, "tags: %ls",buffer->sub_name);
	}else{
		mvprintw(row, col, "tags: %ls",buffer->sub_name);
	}

	if(!editing_tags && !only_tag_str ){
		buffer->cursor_col=col+ sizeof("name: ")+buffer->local_curs_pos-1;
		buffer->cursor_row=row;
	}else{
		buffer->cursor_col=col+ sizeof("name: ")+buffer->local_curs_pos-1;
		if(!only_tag_str)
			buffer->cursor_row=row+1;
		else
			buffer->cursor_row=row;
	}
}

