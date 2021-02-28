#include <string.h>
#include <ncurses.h>
#include <wchar.h>
#include "trlg_common.h"
#include "trlg_string.h"

wchar_t* get_after_last_comma (wchar_t* str){
	int last_comma_pos=0;
	for(int i=0;i<wcslen(str);i++){
		if(str[i]==','){
			last_comma_pos=i+1;
		}
	}
	return str+last_comma_pos;
}

int remove_wchar(wchar_t* str,int index){
	if(str==0){
		return 1;
	}

	int len=wcslen(str);

	if(len==0){
		return 1;
	}

	for(int i=index;i<len;i++){
		str[i]=str[i+1];
	}
	return 0;
}

wchar_t* remove_spaces(wchar_t* str){
	int len=wcslen(str);
	for(int i=0;i<len;i++){
		if(str[i]==' '){
			memcpy(str+i, str+i+1,len-i);
			i--;
		}
	}
	return str;
}

void remove_commas_from_end(wchar_t* str){
	while(str[wcslen(str)-1]==','){
		str[wcslen(str)-1]=0;
	}
}

char last_char(char* str){
	int len=strlen(str);
	if(len==0) 
		return 0;
	return str[strlen(str)-1];
}

char last_wchar(wchar_t* str){
	int len=wcslen(str);
	if(len==0) 
		return 0;
	return str[wcslen(str)-1];
}

char* next_comma(char* str){
	for(int i=0;i<strlen(str);i++){
		if(str[i]==','){
			return str+i;
		}
	}
	return 0;
}

char char_at(int row,int col){
	chtype a=mvinch(row,col);
	char ret= A_CHARTEXT & a;
	return ret;
}

void print_warp_str(int row, int col,char* str, int len){
	int str_len=strlen(str);
	if(str_len>=len){
		char tmp_str[len];
		char rest_str[str_len];
		memset(tmp_str, 0, str_len);
		memset(rest_str, 0, str_len);
		memcpy(tmp_str,str,str_len);
		tmp_str[len]=0;
		mvprintw(row,col,"%s",tmp_str);
		printw("-");
		memcpy(rest_str,str+len,str_len-len);
		if(char_at(row+1, col+1)==' ' || char_at(row+1, col+1)=='_'){
			print_warp_str(row+1, col, rest_str, len);
		}else{
			move(row,col+strlen(tmp_str)+1);
			return;
		}
	}else{
		mvprintw(row,col,"%s",str);
	}
}

void print_chopoff(int row, int col,wchar_t* str, int len){
	if (len<0) return;
	int str_len=wcslen(str);
	if(str_len>=len){
		wchar_t tmp_str[len];
		wmemcpy(tmp_str,str,len);
		tmp_str[len]=0;
		mvprintw(row,col,"%ls",tmp_str);
		printw("-");
		return;
	}else{
		mvprintw(row,col,"%ls",str);
	}
}

int add_chr_in_wstr(wchar_t chr, wchar_t* str,int index,int max_size){
	int strln=wcslen(str);

	if(index>=max_size){
		return -1;
	}else if (index<0){
		return -1;
	}

	if(strln+2<max_size){

		for(int i=max_size-2;i>=index;i--){
			str[i+1]=str[i];
		}

		str[index]=chr;

		if(index>=strln){
			str[index+1]=0;
			for(int i=index;i>=0;i--){
				if(str[i]==0)str[i]=32;
			}
		}
	}else{
		return -1;
	}
	return 0;
}

int add_chr_in_str(char chr, char* str,int index,int max_size){
	int strln=strlen(str);

	if(index>=max_size){
		return -1;
	}else if (index<0){
		return -1;
	}

	if(strln+2<max_size){
		for(int i=max_size-2;i>=index;i--){
			str[i+1]=str[i];
		}

		str[index]=chr;

		if(index>=strln){
			str[index+1]=0;
			for(int i=index;i>=0;i--){
				if(str[i]==0)str[i]=32;
			}
		}
	}else{
		return -1;
	}
	return 0;
}

strPart get_nth_strpart(wchar_t* str, wchar_t chr, int n){
	strPart part;

	int str_len=wcslen(str);

	int matched=-1;
	wchar_t* last_start=str;

	for(int i=0;i<str_len;i++){
		if(str[i]==chr || i == str_len-1){
			part.start=last_start;
			part.length=&str[i]-last_start+ (i == str_len-1);
			last_start=&str[i+1];
			matched++;
			if(matched==n){
				return part;
			}
		}
	}

	if(matched!=n){
		part.start=0;
		part.length=0;
	}

	return part;
}
