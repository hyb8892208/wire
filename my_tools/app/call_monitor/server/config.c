#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"


static void trim_string(char *str)
{
	char *start, *end;
	int len = strlen(str);

	if(str[len-1] == '\n')
	{
		len--;
		str[len] = 0;
	}
	start = str;
	end = str + len -1;
	while(*start && *start == ' ' )
		start++;
	while(*end && *end == ' ')
		*end-- = 0;	
	strcpy(str, start);
}

static void del_line_by_line(struct section *sec, struct line *line)
{
	//printf("sec:[%s]\n", sec->context);
	if(!line)
		return;
	if(line == sec->first){
		//printf("==== del====  %s=%s\n", line->key?line->key:"", line->value?line->value:"");
		sec->first = line->next;	
		if(sec->first == NULL)
			sec->last = NULL;
		else
			line->next->prev = NULL;
	}else{
		line->prev->next = line->next;
		if(line->next == NULL)
			sec->last = line->prev;
		else
			line->next->prev = line->prev;
	}
	if(line->key){
		free(line->key);
		line->key = NULL;
	}
	if(line->value){
		free(line->value);
		line->value = NULL;
	}
	free(line);
	line = NULL;
	return ;
}

struct line *find_line(struct section *sec, const char *key)
{
	struct line *line = sec->first;
	for(line; line; line = line->next){
		if(line->key && strcmp(line->key, key) == 0){
	//		printf("foud, value = %s\n", line->value);
			return line;
		}
	}
	return NULL;
}

int del_line_by_key(struct config *config, const char *context, const char *key)
{
	struct line *line = line;
	struct section * sec = find_section(config, context);
	if(!sec)
		return -1;
	line = find_line(sec, key);
	if(line)
		del_line_by_line(sec, line);
}

int edit_line_value(struct config *config, const char *context, const char *key, const char *value)
{
	struct line *line = line;
	struct section * sec = find_section(config, context);
	if(!sec)
		return -1;
	line = find_line(sec, key);
	if(line){
		if(line->value){
			free(line->value);
			line->value = strdup(value);
		}
	}else
		return -1;

	return 0;
}

int get_line_value(struct config *config, const char *context, const char *key, char *value)
{
	struct line *line = line;
	struct section * sec = find_section(config, context);
	if(!sec){
	//	printf("not foud %s\n", context);
		return -1;
	}
	value[0] = '\0';
//	printf("foud[%s]\n", context);
	line = find_line(sec, key);
	if(line){
		if(line->value){
			strcpy(value, line->value);
		}else
			return -1;
	}else
		return -1;
	return 0;
}

int destory_line(struct section *sec)
{
	struct line *line = sec->first;
	while(line){
		del_line_by_line(sec, line);
		line = sec->first;
	}
	return 0;
}

struct section *find_section(struct config *config, const char *context)
{
	struct section *sec = config->first;
	for(sec; sec ; sec = sec->next){
		//printf("context=%s, sec->context=%s\n", sec->context, context);
		if(strcmp(sec->context, context) == 0)
			return sec;
	}
	return NULL;
}

struct section *add_section_tail(struct config *config, const char *context)
{
	struct section *tmp = (struct section *)malloc(sizeof(struct section));
	if(!tmp){
		return NULL;
	}
	tmp->first = NULL;
	tmp->last = NULL;	
	
	tmp->context = strdup(context);
	if(tmp->context == NULL){
	//	printf("context is null\n");
	}else{
	//	printf("context is not null\n");
	}
	if(config->first == NULL){
		//printf("none\n");
		config->first = tmp;
		config->last = tmp;
		config->last->next = NULL;
		config->last->prev = NULL;
	}else{
		tmp->next = NULL;
		tmp->prev = config->last;
		config->last->next = tmp;
		config->last = tmp;
	}
	return  tmp;
}

int del_section_by_section(struct config *config, struct section *sec){
	if(sec == config->first){
		config->first = config->first->next;
		if(config->first == NULL)
			config->last = NULL;
		else
			sec->next->prev = NULL;
	}else{
		sec->prev->next = sec->next;
		if(sec->next = NULL)
			config->last = NULL;
		else
			sec->next->prev = sec->prev;
	}
	destory_line(sec);
	free(sec);
	return 0;

}

int del_section(struct config *config, const char *context)
{
	struct section *sec;
	sec = find_section(config,context);
	if(sec){
		del_section_by_section(config, sec);
	}
	return 0;
}

int edit_section(struct config *config, const char *old_context, const char *new_context)
{
	struct section *tmp;
	int ret = -1;
	tmp = find_section(config, old_context);
	if(tmp){
		free(tmp->context);
		tmp->context = strdup(new_context);
		ret = 0;
	}
	return ret;
}

int destory_sections(struct config *config){
	struct section *sec;
	sec = config->last;
	while(sec){
		//destroy line
		del_section_by_section(config, sec);
		sec = config->last;
	}
	return 0;
}

int insert_line_tail_by_line(struct section *sec, struct line *line)
{
	if(sec->last == NULL){
		sec->first = line;
		sec->last = line;
		sec->last->prev = NULL;
		sec->last->next = NULL;
	}else{
		line->prev = sec->last;
		line->next = NULL;
		sec->last->next = line;
		sec->last = line;
	}
	return 0;
}

int insert_key_value_line(struct config *config, const char *context, const char *key, const char *value)
{	
	struct line *line = NULL;
	struct section *sec = NULL;
	sec = find_section(config, context);
	if(!sec){
		sec = add_section_tail(config, context);
	}

	line = (struct line *)malloc(sizeof(struct line));
	//printf("%s = %s\n", key, value);

	line->key = strdup(key);
	line->value = strdup(value);
	insert_line_tail_by_line(sec, line);
	return 0;
}

int insert_comment_line(struct config *config, const char *context, const char *key)
{
	struct line *line = NULL;
	struct section *sec = NULL;
	sec = find_section(config, context);
	if(!sec){
		sec = add_section_tail(config, context);
	}

	line = (struct line *)malloc(sizeof(struct line));

	line->key = strdup(key);
	line->value = NULL;
	insert_line_tail_by_line(sec, line);
	return 0;

}

int insert_spare_line(struct config *config, const char *context)
{
	struct line *line = NULL;
	struct section *sec = NULL;
	sec = find_section(config, context);
	if(!sec){
		sec = add_section_tail(config, context);
	}

	line = (struct line *)malloc(sizeof(struct line));
	line->value = NULL;
	line->key = NULL;
	insert_line_tail_by_line(sec, line);
	return 0;
}

int insert_value_line(struct config *config, const char *context, const char *value)
{
	struct line *line = NULL;
	struct section *sec = NULL;
	sec = find_section(config, context);
	if(!sec){
		sec = add_section_tail(config, context);
	}

	line = (struct line *)malloc(sizeof(struct line));

	line->value = strdup(value);
	line->key = NULL;
	insert_line_tail_by_line(sec, line);
	return 0;
}

enum line_type parese_line(char *str, char *sec, char *key, char *value){
	trim_string(str);
	enum line_type line_type;
	if(strlen(str) == 0)
		line_type = LINE_SPARE;
	else if(str[0] == '#' || str[0] == ';'){
		line_type = LINE_COMMENT;
		strcpy(key, str);
	}else if(str[0] == '['){
//		printf("pares section line\n");
		line_type = LINE_SECTION;
		sscanf(str, "[%[^]]", sec);
	}else if(strstr(str, "=")){
	//	printf("key value line\n");
//		sscanf(str, "%[^=] = \"%[^\"]\"", key, value);
	//	sscanf(str, "%[^=]=%s", key, value);
		sscanf(str, "%[^=!]%*[=!]%[^?]%*[£¡?]%s",key, value);
//		char * s = strstr(str, "=");
//		int pos = s - str;
//		char *pos = strstr(str, "=");
//		*(pos+1)='\0';
	//	str[pos] = '\0';
//		strcpy(key, str);
//		strcpy(value, pos+2);
	//	key = str;
	//	value = pos + 2;
//		printf("||||||||||%s=%s||||||||\n", key, value);
		line_type = LINE_KEY_VALUE;
	}else{
		strcpy(value, str);
		line_type = LINE_VALUE;
	}
//	printf("line_type=%d\n", line_type);
	return line_type;
}

int load_config(char *filename, struct config *config)
{
	char sec[1024] = {0};
	char key[1024] = {0};
	char val[1024] = {0};
	char *line_buf;
	enum line_type line_type;
	size_t size=0;
	FILE  *handle = NULL;
	
	strncpy(config->filename, filename, sizeof(config->filename));
	printf("%s\n", config->filename);

	config->first = config->last = NULL;
	handle = fopen(filename, "r");
	if(handle == NULL)
		return -1;
	while(getline(&line_buf, &size, handle) > 0){
//		printf("line_buf=%s\n", line_buf);
		line_type = parese_line(line_buf, sec, key, val);
//		printf("line_type=%d\n",line_type);
		switch (line_type)
		{
			case LINE_SECTION:
				add_section_tail(config, sec);
				break;
			case LINE_KEY_VALUE:
				insert_key_value_line(config,sec, key, val);
				break;
			case LINE_COMMENT:
				insert_comment_line(config, sec, key);
				break;
			case LINE_VALUE:
				insert_value_line(config, sec, val);
				break;
			case LINE_SPARE:
				insert_spare_line(config, sec);
				break;
			default:
				printf("unkonw type\n");
				break;
		}
	}
	free(line_buf);
	fclose(handle);
	return 0;
}

int save_config(struct config *config){
	struct section *sec;
	struct line *line;
	sec = config->first;
	for(sec; sec != NULL; sec = sec->next){
		if(strlen(sec->context) > 0)
			printf("[%s]\n",sec->context);
		line = sec->first;
		for(line; line; line = line->next){
			if(line->value && line->key){
				printf("%s=%s\n", line->key, line->value);
			}else if(line->value){
				printf("%s\n", line->value);
			}else if(line->key){
				printf("%s\n", line->key);
			}else{
				printf("\n");
			}
		}
	}
	return 0;
}

int config_deinit(struct config *config)
{
	struct section *sec = config->first;
	while(sec){
		del_section_by_section(config, sec);
		sec = config->first;
	}
	return 0;
}

//#define __TEST__
#ifdef __TEST__


int main(int argc, char **argv)
{
	/*
	 * argv[0] filename context key value
	 * */
	char value[128];
	struct config test_conf;
	load_config( argv[1], &test_conf);

	save_config(&test_conf);
	get_line_value(&test_conf, argv[2], argv[3], value);
	printf("===================\n");
	printf("%s=%s\n", argv[3], value);
	config_deinit(&test_conf);
	return 0;
}

#endif
