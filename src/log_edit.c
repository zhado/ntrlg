#include <ncurses.h>
#include <string.h>

#include "trlg_common.h"
#include "logs.h"
#include "log_edit.h"
#include "trlg_string.h"

log_edit_buffer init_log_edit(t_log* a_log, bool only_tag_str, wchar_t* name, wchar_t* tags){
	log_edit_buffer buffer;
	wmemset(buffer.name, 0, MAX_NAME_SIZE);
	wmemset(buffer.tags, 0, MAX_NAME_SIZE);
	if(name!=0 && !only_tag_str) wcscpy(buffer.name, name);
	if(tags!=0)wcscpy(buffer.tags, tags);
	buffer.a_log=a_log;
	buffer.tag_autocomp_selection=-1;
	buffer.only_tag_str=only_tag_str;
	buffer.matched_count=0;
	buffer.cursor_row=0;
	buffer.cursor_col=0;
	if(!only_tag_str){
		buffer.local_curs_pos=wcslen(buffer.name);
	}else{
		buffer.local_curs_pos=wcslen(buffer.tags);
	}

	return buffer;
}

int log_edit(log_edit_buffer* buffer,t_log* log_p, wchar_t chr){
	int* autocomp_selection=&buffer->tag_autocomp_selection;
	wchar_t* name=buffer->name;
	wchar_t* tag_str=buffer->tags;
	t_log* a_log=buffer->a_log;
	bool only_tag_str=buffer->only_tag_str;
	bool editing_tags=false;

	int selected_id=0;
	int requested_str_size=0;
	wchar_t* requested_str=0;

	if(*autocomp_selection!=-1){
		selected_id=buffer->sni[*autocomp_selection].tag_id;
		requested_str=log_p->tg_enrtries[selected_id].tag;
		requested_str_size=wcslen(log_p->tg_enrtries[selected_id].tag);
	}

	if(buffer->name[wcslen(buffer->name)-1]==10){
		editing_tags=true;
	}

	if((chr > 31  && chr != 127 ) && !(chr > 257 && chr < 262)){
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
			if(remove_wchar(buffer->name, buffer->local_curs_pos-1)==0){
				buffer->local_curs_pos--;
			}
		}else{
			if(remove_wchar(buffer->tags, buffer->local_curs_pos-1)==0){
				buffer->local_curs_pos--;
			}
		}
	}else if (chr == KEY_UP && (editing_tags || only_tag_str) ){
		if(*autocomp_selection< buffer->matched_count-1)
			*autocomp_selection=*autocomp_selection+1;
	}else if (chr == KEY_DOWN && (editing_tags || only_tag_str)){
		if(*autocomp_selection>-1)
			*autocomp_selection=*autocomp_selection-1;
	}else if (chr == KEY_RIGHT ){
		if( (!editing_tags || only_tag_str) && (wcslen(buffer->tags) >buffer->local_curs_pos)){
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
			buffer->local_curs_pos=wcslen(buffer->tags);
		}else{
			if(*autocomp_selection==-1){
				if(!only_tag_str)
					name[wcslen(name)-1]=0;
				return 0;
			}else{
				if(get_after_last_comma(tag_str)!=tag_str || only_tag_str)
					wmemcpy(tag_str+tag_str_len-wcslen(get_after_last_comma(tag_str))+1*(tag_str[wcslen(tag_str)-1]==32),
							requested_str,requested_str_size);
				else
					wmemcpy(tag_str+wcslen(tag_str)-wcslen(get_after_last_comma(tag_str)),
							requested_str,requested_str_size);
				tag_str[wcslen(tag_str)]=',';
				tag_str[wcslen(tag_str)]=' ';
				buffer->local_curs_pos=wcslen(tag_str);
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
	wchar_t* tag_str=buffer->tags;
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
		mvprintw(row+1, col, "tags: %ls",buffer->tags);
	}else{
		mvprintw(row, col, "tags: %ls",buffer->tags);
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

