#ifndef TRLG_STRING_H 
#define TRLG_STRING_H

char* get_after_last_comma (char* str);
char* remove_spaces(char* str);
char last_char(char* str);
char* next_comma(char* str);
void print_warp_str(int row, int col,char* str, int len);
char char_at(int row,int col);
int add_chr_in_str(char chr, char* str,int index,int max_size);

#endif
