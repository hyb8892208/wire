
#ifndef __CLI_H_
#define __CLI_H_

typedef int (*pFUNC_CALLBACK)(int argc, char **argv);   /* �����лص����� */

int cb_func_reg(char *cmd, pFUNC_CALLBACK pFunc);

int run_main(char *propt);

void cli_init(void);

#endif

