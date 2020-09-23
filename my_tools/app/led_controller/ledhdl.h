#ifndef _LEDHDL_H
#define _LEDHDL_H


/* 配置串口参数 */
void m_setparms(int fd, char *baudr, char *par, char *bits, char *stopb, int hwf, int swf);

/* 打开串口 */
int com_open(char *dev_name);

/* 向串口中写入数据 */
int com_write(int fd, char *buf, int len);

int com_read(int fd, char *buf, int len);


/* 关闭串口 */
void com_close(int fd);

#endif
