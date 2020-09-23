#include "../include/utf8_simple.h"

/**************************************************************
编码原理	https://zh.wikipedia.org/wiki/UTF-8
　　UCS-4 range (hex.) UTF-8 octet sequence (binary)
　　0000 0000-0000 007F 0xxxxxxx
　　0000 0080-0000 07FF 110xxxxx 10xxxxxx
　　0000 0800-0000 FFFF 1110xxxx 10xxxxxx 10xxxxxx
　　0001 0000-001F FFFF 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
　　0020 0000-03FF FFFF 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
　　0400 0000-7FFF FFFF 1111110x 10xxxxxx ... 10xxxxxx

	utf8字符长度1-6，可以根据每个字符第一个字节判断整个字符长度
***************************************************************/

//定义查找表，长度256，表中的数值表示以此为起始字节的utf8字符长度
static unsigned char utf8_look_for_table[256] = 
{
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,I  ,
	II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,
	II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,II ,
	III,III,III,III,III,III,III,III,III,III,III,III,III,III,III,III,
	IV ,IV ,IV ,IV ,IV ,IV ,IV ,IV ,V  ,V  ,V  ,V  ,VI ,VI ,I  ,I
};


#define UTFLEN(x)  utf8_look_for_table[(x)]

int get_char_utf_len(unsigned char ch)
{
	return UTFLEN(ch);
}

int get_utf8_length(unsigned char *str,int clen)
{
	int len = 0;
	unsigned char *ptr;

	for(ptr = str; *ptr!=0&&len<clen; len++){
		ptr+=UTFLEN(*ptr);
	}
	return len;
}

