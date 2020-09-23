#include "serial.h"
//#include "msg.h"


// baud map table
baudmap_t baud_table[] = 
{
	{ 1200,   B1200 },
	{ 2400,   B2400 },
	{ 4800,   B4800 },
	{ 9600,   B9600 },
	{ 19200,  B19200 },
	{ 38400,  B38400 },
	{ 57600,  B57600 },
	{ 115200, B115200 },
	{ 230400, B230400 },
	{ 460800, B460800 },
	{ 921600, B921600 },
};

#define NMBAUDS (sizeof(baud_table) / sizeof(baudmap_t))


// find relevant map flag
static sint32 baud2flag(uint32 baud)
{
	uint32	i = 0;
	for (; i < NMBAUDS; ++i)
	{
		if (baud == baud_table[i].baud)
		{
			return baud_table[i].flag;
		}
	}
	return SERIAL_ERR;
}


/******************************************************************************
Function	:	open_serial
Description	:	打开串口
Input		:	serial_file		串口文件
Return		:	成功：返回串口描述符 失败：返回：-1
******************************************************************************/
sint32 open_serial_handle(char *serial_file)
{
	int fd = -1;
	fd = open(serial_file, (O_RDWR | O_NDELAY | O_NONBLOCK));
	if (fd < 0)
	{
		return SERIAL_ERR;
	}
	return fd;
}

/******************************************************************************
Function	:	open_serial
Description	:	打开串口
Input		:	serial_file		串口文件
Return		:	成功：返回串口描述符 失败：返回：-1
******************************************************************************/
sint32 open_serial(char *serial_file, uint32 rate, uint8 parity, uint8 databits, uint8 stopbits, uint8 streamcontrol)
{
	int fd = -1;
	int ret = -1;
	serial_attr_t attr;
	struct termios term;

	fd = open_serial_handle(serial_file);
	if (fd < 0)
	{
		return SERIAL_ERR;
	}

	//
	memset(&term, 0, sizeof(term));
	ret = get_termios(fd, &term);
	if (ret != 0)
	{
		close_serial(fd);
		return -1;
	}

	//
	attr.rate = rate;
	attr.parity = parity;
	attr.databits = databits;
	attr.stopbits = stopbits;
	attr.streamcontrol = streamcontrol;

	ret = set_serial_attr(fd, &attr);
	if (ret != 0)
	{
		printf("set_serial_attr error\n");
		close_serial(fd);
		return -1;
	}

	return fd;
}

/******************************************************************************
Function	:	close_serial
Description	:	关闭串口
Input		:	serial_fd		串口文件描述符
Return		:	成功：返回0 失败：返回：-1
******************************************************************************/
sint32 close_serial(uint32 serial_fd)
{
	close(serial_fd);
	return SERIAL_SUCC;
}

/******************************************************************************
Function	:	get_termios
Description	:	获取串口termios
Input		:	serial_fd		串口文件描述符
				old_term		串口termios数据结构
Return		:	成功：返回0 失败：返回：-1
******************************************************************************/
sint32 get_termios(uint32 serial_fd, struct termios *old_term)
{
	sint32 ret = 0;
	ret = tcgetattr(serial_fd, old_term);
	if (ret < 0)
	{
		return SERIAL_ERR;
	}
	return SERIAL_SUCC;
}

/******************************************************************************
Function	:	set_termios
Description	:	设置串口termios
Input		:	serial_fd		串口文件描述符
				old_term		串口termios数据结构
Return		:	成功：返回0 失败：返回：-1
******************************************************************************/
sint32 set_termios(uint32 serial_fd, struct termios *old_term)
{
	sint32 ret = 0;
	ret = tcsetattr(serial_fd, TCSAFLUSH, old_term);
	if (ret < 0)
	{
		return SERIAL_ERR;
	}
	return SERIAL_SUCC;
}

/******************************************************************************
Function	:	set_serial_attr
Description	:	设置串口属性
Input		:	serial_fd		串口文件描述符
				serial_attr		串口属性数据结构
Return		:	成功：返回0 失败：返回：-1
******************************************************************************/
sint32 set_serial_attr(uint32 serial_fd, serial_attr_t *serial_attr)
{
	uint32 clocal = 1;
	struct termios tio;

	if (serial_attr == NULL)
	{
		return SERIAL_ERR;
	}
	
	memset(&tio, 0, sizeof(tio));
	
	tio.c_cflag = CREAD | HUPCL | baud2flag(serial_attr->rate);	
	tio.c_cflag |= CBAUDEX;
	if (clocal)
	{
		tio.c_cflag |= CLOCAL;
	}
	switch (serial_attr->parity)
	{
	case 1:
		{
			tio.c_cflag |= PARENB | PARODD;
		}
		break;
	case 2:
		{
			tio.c_cflag |= PARENB;
		}
	default:
		break;
	}
	switch (serial_attr->databits)
	{
	case 5:
		{
			tio.c_cflag |= CS5;
		}
		break;
	case 6:
		{
			tio.c_cflag |= CS6;
		}
		break;
	case 7:
		{
			tio.c_cflag |= CS7;
		}
		break;
	default:
		{
			tio.c_cflag |= CS8;
		}
		break;
	}

	if (serial_attr->streamcontrol == 2)
	{
		tio.c_iflag |= IXON | IXOFF;
	}
	if (serial_attr->streamcontrol == 1)
	{
		tio.c_cflag |= CRTSCTS;
	}

	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;
	if (tcsetattr(serial_fd, TCSANOW, &tio) < 0)
	{
		return SERIAL_ERR;
	}
	return SERIAL_SUCC;
}

/******************************************************************************
Function	:	serial_read_n
Description	:	读串口
Input		:	serial_fd		串口文件描述符
				buf				数据缓存
				len				读出长度
				timeout			读超时,单位:秒
Return		:	0				返回读出数据长度
				-1				等待失败
				-2				等待超时
				-3				读取失败
******************************************************************************/
#if 1
sint32 serial_read_n(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout)
{
	sint32 ret = 0;
	uint32 readn = 0;
	fd_set fds;
	struct timeval to;

	if (buf == NULL)
	{
		return SERIAL_ERR;
	}
	to.tv_sec = 0;
	to.tv_usec = 600000; //10000; //50000;//timeout * 1000000; // change to 100000 for pty

	//for (readn = 0; readn < len; readn++)
	while (readn < len)
	{
		FD_ZERO(&fds);
		FD_SET(serial_fd, &fds);
		ret = select(serial_fd+1, &fds, 0, 0, &to);
		if (ret <= 0)
		{
			return readn;
		}
		else
		{
			ret = read(serial_fd, buf+readn, len-readn);
			if (ret < 0)
			{
				return readn;
			}
			readn += ret;
		}
		//logDataToHex(buf, (unsigned short)readn, 0, 0, (unsigned short)buf[0], DIRE_COMM_TO_COMMHDLTASK);
	}

	return readn;
}
#else
sint32 serial_read_n(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout)
{
	sint32 ret = 0;
	uint32 readn = 0;
	fd_set fds;
	struct timeval to;

	if (buf == NULL)
	{
		return SERIAL_ERR;
	}
	to.tv_sec = 0;
	to.tv_usec = 600000; //10000; //50000;//timeout * 1000000; // change to 100000 for pty

	for (readn = 0; readn < len; readn++)
	{
		FD_ZERO(&fds);
		FD_SET(serial_fd, &fds);
		ret = select(serial_fd+1, &fds, 0, 0, &to);
		if (ret <= 0)
		{
			return readn;
		}
		else
		{
			ret = read(serial_fd, buf+readn, 1);
			if (ret != 1)
			{
				return readn;
			}
		}
	}

	return readn;
}
#endif


sint32 serial_read_n_atr(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout)
{
	sint32 ret = 0;
	uint32 readn = 0;
	fd_set fds;
	struct timeval to;

	if (buf == NULL)
	{
		return SERIAL_ERR;
	}
	to.tv_sec = 1; //0;
	to.tv_usec = 500000; //50000; //50000;//timeout * 1000000; // change to 100000 for pty

	for (readn = 0; readn < len; readn++)
	{
		FD_ZERO(&fds);
		FD_SET(serial_fd, &fds);
		ret = select(serial_fd+1, &fds, 0, 0, &to);
		if (ret < 0)
		{
			return SERIAL_ERR;
		}
		else if (ret == 0)
		{
			return readn;
		}
		else
		{
			ret = read(serial_fd, buf+readn, 1);
			if (ret != 1)
			{
				return readn;
			}
		}
	}

	return readn;
}

/******************************************************************************
Function	:	serial_write_n
Description	:	写串口
Input		:	serial_fd		串口文件描述符
				buf				数据缓存
				len				写入长度
				timeout			写超时
Return		:	成功：返回写入数据长度 失败：返回：-1
******************************************************************************/
#if 0
sint32 serial_write_n(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout)
{
	sint32 ret = 0;
	uint32 sendn = 0;
	uint32 i = 0;
	fd_set fds;
	struct timeval to;

	if (buf == NULL)
	{
		return -1;
	}

	to.tv_sec = 0;
	to.tv_usec = timeout * 1000000;

	for (i = 0; i < len; i++)
	{
		FD_ZERO(&fds);
		FD_SET(serial_fd, &fds);
		ret = select(serial_fd+1, 0, &fds, 0, &to);
		if (ret < 0)
		{
			return SERIAL_ERR;
		}
		else if (ret == 0)
		{
			return sendn;
		}
		else
		{
			ret = write(serial_fd, &buf[i], 1);
			if (ret != 1)
			{
				return sendn;
			}
			sendn++;
		}
	}
	return sendn;
	printf("serial_write_n: len:%d, sendn:%d\n", len, sendn);
}
#else
sint32 serial_write_n(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout)
{
	sint32 ret = 0;
	uint32 sendn = 0;
	//uint32 i = 0;
	//fd_set fds;
	//struct timeval to;

	if (serial_fd < 0 || buf == NULL)
	{
		return -1;
	}

	/*for (i = 0; i < len; i++)
	{
		ret = write(serial_fd, &buf[i], 1);
		if (ret != 1)
		{
			zprintf("[ERROR]serial_write_n(len:%d, sendn:%d) error(%d:%s)", len, sendn, errno, strerror(errno));
			return sendn;
		}
		sendn++;
	}*/
	while (sendn < len)
        {
                ret = write(serial_fd, buf+sendn, len-sendn);
                if (ret < 0)
                        return sendn;
                sendn += ret;
        }
	return sendn;
}
#endif

/******************************************************************************
Function	:	serial_wr
Description	:	读写串口
Input		:	serial_fd		串口文件描述符
				buf				数据缓存
				len				写入长度
				timeout			写超时
Return		:	成功：返回写入数据长度 失败：返回：-1
******************************************************************************/
#if 1
#if 0
#if 0
sint32 serial_wr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
	int ret = -1;
	int len = 0;

	if (serial_fd < 0 || buf_w == NULL || buf_r == NULL)
	{
		return -1;
	}
	zprintf("[INFO]serial_wr: before serial_write_n [len_w:%d]......... ......... .........", len_w);
	ret = serial_write_n(serial_fd, buf_w, len_w, timeout);
	zprintf("[INFO]serial_wr: serial_write_n done [ret:%d]......... ......... .........", ret);
	if (ret != len_w)
	{
		zprintf("[ERRO]serial_wr:1  write[len_w:%d, ret:%d] to serial error(%d:%s) .........", len_w, ret, errno, strerror(errno));
		return -1;
	}

	
	fd_set fds;
	struct timeval to;
	to.tv_sec = 0;
	to.tv_usec = 6000; //10000; //50000;//timeout * 1000000; // change to 100000 for pty
	FD_ZERO(&fds);
	FD_SET(serial_fd, &fds);
	ret = select(0, 0, 0, 0, &to);
	//if (ret <= 0)
	//{
	//	return 0;
	//}
	ret = read(serial_fd, buf_r, len_r);
	zprintf("[INFO]serial_wr: read done [ret:%d]......... ......... .........", ret);
	return ret;
}
#else
sint32 serial_wr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
	int ret = -1;
	int len = 0;

	if (serial_fd < 0 || buf_w == NULL || buf_r == NULL)
	{
		return -1;
	}
	//zprintf("[INFO]serial_wr: current 1 [len_w:%d]......... ......... .........", len_w);
	ret = serial_write_n(serial_fd, buf_w, len_w, timeout);
	//zprintf("[INFO]serial_wr: current 1 [ret:%d]......... ......... .........", ret);
	if (ret != len_w)
	{
		zprintf("[ERRO]serial_wr:1  write[len_w:%d, ret:%d] to serial error(%d:%s) .........", len_w, ret, errno, strerror(errno));
		return -1;
	}

	
	fd_set fds;
	struct timeval to;
	to.tv_sec = 0;
	to.tv_usec = 600000; //10000; //50000;//timeout * 1000000; // change to 100000 for pty
	FD_ZERO(&fds);
	FD_SET(serial_fd, &fds);
	ret = select(serial_fd+1, &fds, 0, 0, &to);
	if (ret <= 0)
	{
		return 0;
	}
	return read(serial_fd, buf_r, len_r);
}
#endif
#else
sint32 serial_wr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
	int ret = -1;
	int len = 0;

	if (serial_fd < 0 || buf_w == NULL || buf_r == NULL)
	{
		return -1;
	}
	//zprintf("[INFO]serial_wr: current 1 [len_w:%d]......... ......... .........", len_w);
	ret = serial_write_n(serial_fd, buf_w, len_w, timeout);
	//zprintf("[INFO]serial_wr: current 1 [ret:%d]......... ......... .........", ret);
	if (ret != len_w)
	{
		printf("[ERRO]serial_wr:1  write[len_w:%d, ret:%d] to serial error(%d:%s) .........", len_w, ret, errno, strerror(errno));
		return -1;
	}
	// read header from serial
	//zprintf("[INFO]serial_wr: current 2 [len_r:%d]......... ......... .........", 4);
	ret = serial_read_n(serial_fd, buf_r, 4, timeout);
	//zprintf("[INFO]serial_wr: current 2 [ret:%d]......... ......... .........", ret);
	if (ret != 4)
	{
		printf("[ERRO]serial_wr:2  read[len_r:%d, ret:%d] from serial error(%d:%s) .........", 4, ret, errno, strerror(errno));
		return ret;
	}
	// read body from serial
	len = (buf_r[2]<<8) + buf_r[3];
	//len = buf_r[3];
	if (len > 0)
	{
		//zprintf("[INFO]serial_wr: current 3 [len_r:%d]......... ......... .........", len);
		ret = serial_read_n(serial_fd, buf_r+4, len, timeout);
		if (ret != len)
		{
			printf("[ERRO]serial_wr:3  read[len_r:%d, ret:%d] from serial error(%d:%s) .........", len, ret, errno, strerror(errno));
		}
		//zprintf("[INFO]serial_wr: current 3 [ret:%d]......... ......... .........", ret);
		ret += 4;
	}
	return ret;
}
#endif
#else
sint32 serial_wr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
	int ret = -1;

	if (serial_fd < 0 || buf_w == NULL || buf_r == NULL)
	{
		return -1;
	}
	ret = serial_write_n(serial_fd, buf_w, len_w, timeout);
	if (ret != len_w)
	{
		return -1;
	}
	return serial_read_n(serial_fd, buf_r, len_r, timeout);
}
#endif
#if 0
sint32 serial_wr_atr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
	int ret = -1;

	if (serial_fd < 0 || buf_w == NULL || buf_r == NULL)
	{
		return -1;
	}
	zprintf("[INFO]serial_wr: current 1 [len_w:%d]......... ......... .........", len_w);
	ret = serial_write_n(serial_fd, buf_w, len_w, timeout);
	zprintf("[INFO]serial_wr: current 2 [ret:%d]......... ......... .........", ret);
	if (ret != len_w)
	{
		return -1;
	}
	ret = serial_read_n_atr(serial_fd, buf_r, len_r, timeout);
	zprintf("[INFO]serial_wr: current 3 [len_r:%d, ret:%d]......... ......... .........", len_r, ret);
	return ret;
}
#else
#if 1
#if 1
sint32 serial_wr_atr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
	int ret = -1;
	int len = 0;

	if (serial_fd < 0 || buf_w == NULL || buf_r == NULL)
	{
		return -1;
	}
	// write to serial
	ret = serial_write_n(serial_fd, buf_w, len_w, timeout);
	if (ret != len_w)
	{
		printf("[ERRO]serial_wr_atr:1  write[len_w:%d, ret:%d] to serial error(%d:%s) .........", len_w, ret, errno, strerror(errno));
		return -1;
	}
	// read header from serial
	ret = serial_read_n_atr(serial_fd, buf_r, 4, timeout);
	if (ret != 4)
	{
		printf("[ERRO]serial_wr_atr:2  read[len_r:%d, ret:%d] from serial error(%d:%s) .........", 4, ret, errno, strerror(errno));
		return ret;
	}
	// read body from serial
	len = ((int)(buf_r[2])<<8) + (int)(buf_r[3]);
	//len = buf_r[3];
	if (len > 0)
	{
		ret = serial_read_n_atr(serial_fd, buf_r+4, len, timeout);
		if (ret != len)
		{
			printf("[ERRO]serial_wr_atr:3  read[len_r:%d, ret:%d] from serial error(%d:%s) .........", len, ret, errno, strerror(errno));
		}
		ret += 4;
	}
	return ret;
}
#else
sint32 serial_wr_atr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
        int ret = -1;
        int len = 0;

        if (serial_fd < 0 || buf_w == NULL || buf_r == NULL)
        {
                return -1;
        }
        // write to serial
        ret = serial_write_n(serial_fd, buf_w, 1, timeout);
        if (ret != len_w)
        {
          return -1;
        }
        // read header from serial
        ret = serial_read_n_atr(serial_fd, buf_r, 1, timeout);
        if (ret != 1)
        {
           return -1;
        }
	return 0;
}
#endif
#else
sint32 serial_wr_atr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
	int ret = -1;
	int len = 0;
	int i = 0;

	if (serial_fd < 0 || buf_w == NULL || buf_r == NULL)
	{
		return -1;
	}
	for (i = 0; i < 2; i++)
	{
		// write to serial
		ret = serial_write_n(serial_fd, buf_w, len_w, timeout);
		if (ret != len_w)
		{
			if (i == 0)
			{
				continue;
			}
			else
			{
				ret = -1;
				break;
			}
		}
		// read header from serial
		ret = serial_read_n_atr(serial_fd, buf_r, 4, timeout);
		if (ret != 4)
		{
			if (i == 0)
			{
				continue;
			}
			else
			{
				ret = -1;
				break;		
			}
		}
		// read body from serial
		len = buf_r[3];
		if (len > 0)
		{
			ret = serial_read_n_atr(serial_fd, buf_r+4, len, timeout);
			ret += 4;
		}
		if ((buf_w[1]-1) == buf_r[1])
		{
			if (i == 0)
			{
				continue;
			}
		}
	}
	return ret;
}
#endif
int read_from_mini51(uint32 serial_fd, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
	int ret = 0;
	int len = 0;

	if (serial_fd < 0 || buf_r == NULL)
	{
		return -1;
	}

	ret = serial_read_n_atr(serial_fd, buf_r, 3, timeout);
	if (ret != 3)
	{
		return ret;
	}
	// read body from serial
	len = (buf_r[1]<<8) + buf_r[2];
	if (len > 0)
	{
		ret = serial_read_n_atr(serial_fd, buf_r+3, len, timeout);
		ret += 3;
	}
	return ret;
}
int write_to_mini51(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint32 timeout)
{
	int ret = 0;

	ret = serial_write_n(serial_fd, buf_w, len_w, timeout);
	if (ret != len_w)
	{
		return -1;
	}
	return ret;
}
sint32 serial_wr_mini51(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout)
{
	int ret = 0;
	ret = write_to_mini51(serial_fd, buf_w, len_w, timeout);
	if (ret != len_w)
	{
		return -1;
	}
	ret = read_from_mini51(serial_fd, buf_r, len_r, timeout);
	if (ret < 0)
	{
		return -1;
	}
	return 0;
}

#endif
