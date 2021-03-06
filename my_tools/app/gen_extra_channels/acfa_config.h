
#if !defined(_ACFA_CONFIG_H_)
#define _ACFA_CONFIG_H_

// max length of a line in ini file, tail will be discarded;
//#define ACFA_FILEIO_BUF_SIZE (1024*8)	// we use 8K block read/write file to increase speed

// max length of a config line
// Modified by OpenVox,Inc 2015-12-17 14:32
//#define ACFA_MAX_LINE_LEN	(256)
#define ACFA_MAX_LINE_LEN	(2048)

// max length of number data type in internal buffer;
//#define ACFA_MAX_NUMBER_LEN	128


#endif //_ACFA_CONFIG_H_
