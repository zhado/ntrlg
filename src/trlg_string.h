#ifndef TRLG_STRING_H 
#define TRLG_STRING_H

#include <wchar.h>

typedef struct {
	wchar_t* start;
	int length;
	// not including null pointer if avialable
}strPart;

strPart get_nth_strpart(wchar_t* str, wchar_t chr, int n);
wchar_t* get_after_last_comma (wchar_t* str);
wchar_t* remove_spaces(wchar_t* str);
void remove_commas_from_end(wchar_t* str);
int remove_wchar(wchar_t* str,int index);
char last_wchar(wchar_t* str);
char* next_comma(char* str);
void print_chopoff(int row, int col,wchar_t* str, int len);
char char_at(int row,int col);
int add_chr_in_wstr(wchar_t chr, wchar_t* str,int index,int max_size);
#endif
