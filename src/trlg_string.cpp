#include <string.h>
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
