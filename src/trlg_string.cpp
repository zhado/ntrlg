#include <string.h>
#include <ncurses.h>

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
	int strl_len=strlen(str);
	char tmp_str[strl_len];
	char rest_str[strl_len];
	memset(tmp_str, 0, strl_len);
	memset(rest_str, 0, strl_len);
	memcpy(tmp_str,str,len);
	tmp_str[len-1]=0;
	int tmp_str_len=len;
	if(strl_len>=len){
		mvprintw(row,col,"%s",tmp_str);
		printw("-");
		memcpy(rest_str,str+tmp_str_len,strl_len-tmp_str_len);
		if(char_at(row+1, col)==' ' || char_at(row+1, col)=='_'){
			print_warp_str(row+1, col, rest_str, len);
		}else{
			move(row,col+strlen(tmp_str)+1);
			return;
		}
	}else{
		mvprintw(row,col,"%s",str);
	}
}
