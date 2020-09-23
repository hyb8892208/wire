
#ifndef __OS_WINDOWS_H__
#define __OS_WINDOWS_H__

#ifdef TARGET_OS_WINDOWS

int openport(int uart);
DWORD WriteABuffer(HANDLE file, unsigned char * lpBuf, DWORD dwToWrite);
DWORD ReadABuffer(HANDLE file, unsigned char * lpBuf, DWORD dwToRead);
#endif
#endif  /*__OS_WINDOWS_H__*/
