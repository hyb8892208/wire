#ifndef _UTF8_SIMPLE_H
#define _UTF8_SIMPLE_H

enum
{
	I= 1,
	II,
	III,
	IV,
	V,
	VI
};

int get_utf8_length(unsigned char *str,int clen);

int get_char_utf_len(unsigned char ch);

#endif
