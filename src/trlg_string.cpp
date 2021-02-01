#include <string.h>
#include <ncurses.h>
#include "trlg_common.h"

char* get_after_last_comma (char* str){
	int last_comma_pos=0;
	for(int i=0;i<strlen(str);i++){
		if(str[i]==','){
			last_comma_pos=i+1;
		}
	}
	return str+last_comma_pos;
}

char* remove_spaces(char* str){
	int len=strlen(str);
	for(int i=0;i<len;i++){
		if(str[i]==' '){
			memcpy(str+i, str+i+1,len-i);
			i--;
		}
	}
	return str;
}

char last_char(char* str){
	int len=strlen(str);
	if(len==0) 
		return 0;
	return str[strlen(str)-1];
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
	char tmp_str[str_len];
	char rest_str[str_len];
	memset(tmp_str, 0, str_len);
	memset(rest_str, 0, str_len);
	memcpy(tmp_str,str,str_len);
	tmp_str[str_len]=0;
	if(str_len>=len){
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

int add_chr_in_str(char chr, char* str,int index,int max_size){
	int strln=strlen(str);

	if(strln+2<max_size){
		if(index>=max_size){
			return -1;
		}else if (index<0){
			return -1;
		}
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


bool exact_match_comma(char* str,char* str2){
	char my_str[MAX_NAME_SIZE];
	memset(&my_str,0,MAX_NAME_SIZE);
	strcpy(my_str, str);
	remove_spaces(my_str);

	char temp_str[MAX_NAME_SIZE];
	char* ch_start_p=my_str;

	for(;;){
		memset(&temp_str,0,MAX_NAME_SIZE);

		char* n_comma=next_comma(ch_start_p);
		if(n_comma==0){
			memcpy(temp_str,ch_start_p,strlen(my_str));
		}else{
			memcpy(temp_str,ch_start_p,n_comma-ch_start_p);
		}
		if(temp_str[0]!=0 && strcmp(temp_str, str2)==0 )
			return true;

		if(!n_comma || n_comma==ch_start_p+strlen(ch_start_p))break;
		ch_start_p=n_comma+1;
	}
	return false;
}
