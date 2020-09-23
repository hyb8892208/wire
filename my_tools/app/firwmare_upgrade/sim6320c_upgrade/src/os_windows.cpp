

#define __OS_WINDOWS_CPP_H__

#include "platform_def.h"
#include "os_windows.h"

#ifdef TARGET_OS_WINDOWS

int openport(int uart)
{
	const char PORT_NAME_PREFIX[] = "\\\\.\\COM";
	char pc_comport[32];
	memset(pc_comport,0,sizeof(pc_comport));
	sprintf(pc_comport, "%s%d", PORT_NAME_PREFIX, uart);
	if( !hCom ){
		hCom = CreateFile(pc_comport,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,    // comm devices must be opened w/exclusive-access
			NULL, // no security attributes
			OPEN_EXISTING, // comm devices must use OPEN_EXISTING
			0,
			(HANDLE) NULL  // hTemplate must be NULL for comm devices
			);
		if(hCom == (HANDLE) (0xFFFFFFFF))
			return FALSE;

		DCB	dcbComPort;
		dcbComPort.DCBlength = sizeof(DCB);
		GetCommState(hCom,&dcbComPort);
		
		dcbComPort.fBinary = TRUE;
		dcbComPort.fParity = TRUE;
		dcbComPort.fOutxCtsFlow = TRUE;
		dcbComPort.fOutxDsrFlow = FALSE;
		dcbComPort.fDtrControl = DTR_CONTROL_ENABLE;
		dcbComPort.fDsrSensitivity = FALSE;
		dcbComPort.fTXContinueOnXoff = TRUE;
		dcbComPort.fOutX = FALSE;
		dcbComPort.fInX = FALSE;
		dcbComPort.fErrorChar = FALSE;
		dcbComPort.fNull = FALSE;
		dcbComPort.fRtsControl = RTS_CONTROL_HANDSHAKE;
		dcbComPort.fAbortOnError = FALSE;
		dcbComPort.ByteSize	= 8;
		dcbComPort.Parity		= NOPARITY;
		dcbComPort.StopBits	= ONESTOPBIT;
		dcbComPort.BaudRate = CBR_115200;
		SetCommState(hCom,&dcbComPort);
		
		SetCommMask(hCom, EV_RXCHAR|EV_ERR|EV_BREAK);
		SetupComm(hCom, 64 * 1024, 64 * 1024);
		PurgeComm(hCom, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
			
		COMMTIMEOUTS CommTimeouts;
		memset(&CommTimeouts, 0, sizeof(COMMTIMEOUTS));
		CommTimeouts.ReadIntervalTimeout = MAXDWORD;
		CommTimeouts.ReadTotalTimeoutConstant = 0;
		CommTimeouts.ReadTotalTimeoutMultiplier = 0;
		CommTimeouts.WriteTotalTimeoutConstant = 1000;
		CommTimeouts.WriteTotalTimeoutMultiplier = 0;
			
		SetCommTimeouts(hCom,&CommTimeouts);
		return TRUE;
	}
	return FALSE;

}

DWORD WriteABuffer(HANDLE file, unsigned char * lpBuf, DWORD dwToWrite)
{
	OVERLAPPED osWrite = {0};
	DWORD dwWritten;
	DWORD dwRes;
	BOOL fRes;	
	COMSTAT ComStat;
	DWORD dwErrorFlags;

	osWrite.hEvent = ::CreateEvent(NULL,TRUE,FALSE,NULL);
	if(osWrite.hEvent == NULL)
		return FALSE;

	//	::PurgeComm(file, PURGE_TXCLEAR);
	::ClearCommError(file, &dwErrorFlags, &ComStat); 

	do{		
		if(!::WriteFile(file, lpBuf, dwToWrite, &dwWritten, &osWrite))
		{	
			if(::GetLastError() != ERROR_IO_PENDING) 
				return FALSE;
			else 
			{
				dwRes = ::WaitForSingleObject(osWrite.hEvent, 4000);
				switch(dwRes)
				{
				case WAIT_OBJECT_0:
					if(!::GetOverlappedResult(file, &osWrite, &dwWritten, TRUE))
						fRes = FALSE;
					else
						fRes = TRUE;
					break;
				default:
					fRes=FALSE;
					break;
				}			
			}	
		}
		else
			fRes = TRUE;
		dwToWrite-=dwWritten;
		lpBuf+=dwWritten;
	}while(dwToWrite!=0);
	::CloseHandle(osWrite.hEvent);

	return dwWritten;
}


DWORD ReadABuffer(HANDLE file, unsigned char * lpBuf, DWORD dwToRead)
{
	OVERLAPPED osRead = {0};
	DWORD dwRead;
	DWORD dwRes;
	BOOL fRes;	
	DWORD dwError;
	COMSTAT ComStat;
	DWORD dwErrorFlags;	

	osRead.hEvent = ::CreateEvent(NULL,TRUE,FALSE,NULL);
	if(osRead.hEvent == NULL)
		return FALSE;

	do{
		::ClearCommError(file, &dwErrorFlags, &ComStat); 
	}while(!ComStat.cbInQue);

	if(!ReadFile(file, lpBuf, dwToRead, &dwRead, &osRead))
	{	
		if(::GetLastError() != ERROR_IO_PENDING) 
			fRes=FALSE;
		else 
		{
			dwRes = ::WaitForSingleObject(osRead.hEvent, 10000);
			switch(dwRes)
			{
			case WAIT_OBJECT_0:
				if(!::GetOverlappedResult(file, &osRead, &dwRead, TRUE))
					fRes = FALSE;
				else
					fRes = TRUE;
				break;
			default:
				fRes=FALSE;
				break;
			}			
		}	
	}
	else
		fRes = TRUE;

	::CloseHandle(osRead.hEvent);  

	return dwRead;
}





/*
* code
*/
#endif/*TARGET_OS_WINDOWS*/
