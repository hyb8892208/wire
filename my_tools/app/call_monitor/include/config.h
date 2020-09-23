
#ifndef __CONFIG_H
#define __CONFIG_H

typedef enum line_type{
	LINE_SECTION = 0,
	LINE_SPARE,
	LINE_COMMENT,
	LINE_KEY_VALUE,
	LINE_VALUE,
}LINE_TYPE;

typedef struct line{
	char *key;
	char *value;
	struct line *prev;
	struct line *next;
}line_t;

typedef struct section{
	char *context;
	struct line *first;
	struct line *last;
	struct section *prev;
	struct section *next;
}section_t;

typedef struct config{
	char filename[256];
	struct section *first;
	struct section *last;
}config_t;


int del_line_by_key(struct config *config, const char *context, const char *key);

int edit_line_value(struct config *config, const char *context, const char *key, const char *value);

int get_line_value(struct config *config, const char *context, const char *key, char *value);

int destory_line(struct section *sec);

struct section *find_section(struct config *config, const char *context);

struct section *add_section_tail(struct config *config, const char *context);

int del_section(struct config *config, const char *context);

int edit_section(struct config *config, const char *old_context, const char *new_context);

int destory_sections(struct config *config);

int load_config(char *filename, struct config *config);

int save_config(struct config *config);

int config_deinit(struct config *config);
#endif
