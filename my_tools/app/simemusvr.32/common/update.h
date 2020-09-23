#ifndef UPDATE_H_INCLUDED
#define UPDATE_H_INCLUDED

int astman_update_config_init(char *src_filename, char *dst_filename, int reload);
int astman_update_config_add_action(char *action, char *cat, char *var, char *val, char *match, char *line);
int astman_update_config_execute(struct mansession *s, struct message *m);

#endif //UPDATE_H_INCLUDED
