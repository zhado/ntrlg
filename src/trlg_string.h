#ifndef TRLG_STRING_H 
#define TRLG_STRING_H

struct strPart{
	char* start;
	int length;
	// not including null pointer if avialable
};

char* get_after_last_comma (char* str);
char* remove_spaces(char* str);
void remove_commas_from_end(char* str);
char last_char(char* str);
char* next_comma(char* str);
void print_warp_str(int row, int col,char* str, int len);
char char_at(int row,int col);
int add_chr_in_str(char chr, char* str,int index,int max_size);
bool exact_match_comma(char* str,char* str2);
#endif
