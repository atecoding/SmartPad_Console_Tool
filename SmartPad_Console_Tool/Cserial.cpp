#include <windows.h>
#include <stdio.h>

#include "Cserial.h"

//#define _COMM_DEBUG_
//#define   STX        0x02
//#define   ETX        0x03

#define XON		0x11
#define XOFF	0x13

CSeries::CSeries(void)
{
	m_CommPort = INVALID_HANDLE_VALUE;
}

CSeries::~CSeries(void)
{
}

/*
*函数介绍：打开串口
*入口参数：
*出口参数：
*返回值：
*/


/*清空接收缓冲区*/
void  CSeries::PurgeIn(void)
{
	PurgeComm(m_CommPort, PURGE_RXABORT | PURGE_RXCLEAR);
}
/*清空发送缓冲区*/
void  CSeries::PurgeOut(void)
{
	PurgeComm(m_CommPort, PURGE_TXABORT | PURGE_TXCLEAR);
}



/*查询接收缓冲区*/
int  CSeries::InQueCount(void)
{
	if (m_CommPort<0)
		return ERR_PORTID;
	COMSTAT CT;
	DWORD Error;
	ClearCommError(m_CommPort,&Error, &CT);
	return (CT.cbInQue);
}
/*查询发送缓冲区*/
int  CSeries::OutQueCount(void)
{
	if (m_CommPort<0)
		return ERR_PORTID;
	COMSTAT CT;
	DWORD Error;
	ClearCommError(m_CommPort, &Error, &CT);
	return (CT.cbOutQue);
}

/*关闭当前串口*/
int CSeries::SerClose(void)
{  
	int i;
	//	if (m_CommPort<0)
	//		return ERR_PORTID;
	if (m_CommPort == INVALID_HANDLE_VALUE)
		return ERR_NOTOPEN;
	//关闭串口
	for(i=0;i<3;i++)
	{
		if(CloseHandle (m_CommPort))
		{
			m_CommPort = INVALID_HANDLE_VALUE;
			return 0;
		}
		Sleep(5);
	}			
	return ERR_NOTCLOSE; 
}

int CSeries::SerReset(void)
{

	PurgeComm( m_CommPort, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
	return 0;

}

int CSeries::SerOpen(int PortNo, char * param)
{
	DCB			dcb;
	BOOL		fRetVal ;
	BYTE		i,bSet;
	DWORD		dwError;
	COMMTIMEOUTS  to;
	char baudr[128] = {0};

	TCHAR DevName[10];
	wsprintf(DevName,"\\\\.\\COM%01d",(int)PortNo);

	SerClose();
	if ((m_CommPort=CreateFile(DevName , GENERIC_READ | GENERIC_WRITE  , 0  , 
		NULL , OPEN_EXISTING , NULL , NULL )) == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	// set up DCB
	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb) ;

	strcpy(baudr, (char *)param);

	if(!BuildCommDCBA(baudr, &dcb))
  	{
  		SerClose();
    		return -1;
  	}

  	if(!SetCommState(m_CommPort, &dcb))
  	{
    		SerClose();
    		return(-1);
  	}

	to.ReadIntervalTimeout         = MAXDWORD;
  	to.ReadTotalTimeoutMultiplier  = 0;
  	to.ReadTotalTimeoutConstant    = 0;
  	to.WriteTotalTimeoutMultiplier = 0;
 	to.WriteTotalTimeoutConstant   = 0;

	if(!SetCommTimeouts(m_CommPort, &to) )
  	{
    		SerClose();
    		return(-1);
  	}
	
	return 0;
}

#if 0

int CSeries::SerOpen(int PortNo, long BaudRate, int Parity, int ByteSize)
{
	CString MsgStr;
	DCB			dcb;
	BOOL		fRetVal ;
	BYTE		i,bSet;
	DWORD		dwError;
	COMMTIMEOUTS  to;

	TCHAR DevName[10];
	wsprintf(DevName,"\\\\.\\COM%01d",(int)PortNo);

	SerClose();
	if ((m_CommPort=CreateFile(DevName , GENERIC_READ | GENERIC_WRITE  , 0  , 
		NULL , OPEN_EXISTING , NULL , NULL )) == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	for(i=0; i<5; i++)
	{
		/*设置缓冲区大小*/
		fRetVal = SetupComm(m_CommPort, MAX_BUFFER_IN ,MAX_BUFFER_OUT);
		if (fRetVal)		
		{
			break;
		}
		Sleep(100);
	}

	// purge any information in the buffer
	PurgeComm(m_CommPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

	// set up for overlapped I/O

	GetCommTimeouts(m_CommPort, &to);
	to.ReadIntervalTimeout = 0xFFFFFFFF;
	to.ReadTotalTimeoutMultiplier =0;
	to.ReadTotalTimeoutConstant =0;
	to.WriteTotalTimeoutMultiplier =10;//10ms
	to.WriteTotalTimeoutConstant =5000;//5000ms
	SetCommTimeouts(m_CommPort, &to) ;

	// set up DCB
	dcb.DCBlength = sizeof(DCB) ;

	for(i=0; i<5; i++)
	{
		fRetVal =GetCommState(m_CommPort, &dcb);
		if (fRetVal)
		{
			break;
		}
		Sleep(100);
	}

	if (fRetVal == 0)
	{
		dwError = GetLastError();
		CloseHandle(m_CommPort);
		m_CommPort = INVALID_HANDLE_VALUE;
		return -2;
	}

	dcb.BaudRate=BaudRate;
	dcb.ByteSize=ByteSize;
	dcb.Parity = Parity;
	dcb.StopBits =ONESTOPBIT;//2009-10-29

	// setup hardware flow control

#if 2
	bSet = false;
	dcb.fOutxDsrFlow = bSet ;
	if (bSet)
		dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	else
		dcb.fDtrControl = DTR_CONTROL_ENABLE;

	dcb.fOutxCtsFlow = bSet ;
	if (bSet)
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	else
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

	// setup software flow control

	bSet = false;
	dcb.fInX = dcb.fOutX = bSet ;

	dcb.XonLim = 100 ;
	dcb.XoffLim = 100 ;
	dcb.XonChar = XON;
	dcb.XoffChar = XOFF;

	dcb.fBinary = TRUE;
	if (dcb.Parity == 0)
		dcb.fParity = FALSE;
	else
		dcb.fParity = TRUE;
#else

	/**/
	dcb.fOutxCtsFlow = FALSE;				// No CTS output flow control 
	dcb.fOutxDsrFlow = FALSE;				// No DSR output flow control 
	dcb.fDtrControl = DTR_CONTROL_DISABLE; 
	// DTR flow control type 
	dcb.fDsrSensitivity = FALSE;			// DSR sensitivity 
	dcb.fTXContinueOnXoff = TRUE;			// XOFF continues Tx 
	dcb.fOutX = FALSE;						// No XON/XOFF out flow control 
	dcb.fInX = FALSE;						// No XON/XOFF in flow control 
	dcb.fErrorChar = FALSE;					// Disable error replacement 
	dcb.fNull = FALSE;						// Disable null stripping 
	dcb.fRtsControl = RTS_CONTROL_DISABLE; 
	// RTS flow control 
	dcb.fAbortOnError = FALSE;				// 当串口发生错误，并不终止串口读写
#endif
	// set up DCB
	dcb.DCBlength = sizeof( DCB );

	for(i=0; i<5; i++)
	{
		//Sleep(80);
		fRetVal = SetCommState(m_CommPort, &dcb);
		if (fRetVal)		break;
		Sleep(100);
	}
	if (fRetVal == 0)
	{
		dwError = GetLastError();
		// an error occurred, try to recover
		MsgStr.Format("<CE-%u>,Please Check ComPort", dwError);
		MessageBox(NULL, MsgStr, "SetCommState", MB_OK);
		CloseHandle(m_CommPort);
		m_CommPort = INVALID_HANDLE_VALUE;
		return -3;
	}

	GetCommState(m_CommPort, &dcb ) ;
	//	EscapeCommFunction(m_CommPort, SETDTR ) ;
	PurgeComm(m_CommPort , PURGE_TXABORT | PURGE_RXABORT |
		PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

	return 0;




}
#endif


int CSeries::SerWrite(BYTE *pszBuf, int SendCnt, int TimeOut)
{
	BOOL        fWriteStat ;
	DWORD       dwBytesWritten ;
	DWORD       dwErrorFlags;
	DWORD       dwBytesSent=0;
	COMSTAT		ComStat;


	if(SendCnt==0) return 0;

	fWriteStat = WriteFile(m_CommPort, pszBuf, 1, &dwBytesWritten, NULL); 

	if (!fWriteStat)
	{
		// some other error occurred
		ClearCommError(m_CommPort, &dwErrorFlags, &ComStat );

		return -1;
	}
	if(SendCnt>1)
	{

		fWriteStat = WriteFile(m_CommPort, pszBuf+1, SendCnt-1,
			&dwBytesWritten, NULL); 
		if (!fWriteStat)
		{
			// some other error occurred
			ClearCommError(m_CommPort, &dwErrorFlags, &ComStat );
			return -1;
		}
	} 
	return 0;
}

int CSeries::SerRead(BYTE *pszBuf, int RecvCnt, int TimeOut)
{
	BOOL        fReadStat;
	DWORD       OldCount,Count,dwReaded, dwError;
	DWORD       dwErrorFlags;
	COMSTAT		ComStat;
	DWORD		i=0;

	OldCount=GetTickCount();
	while (1)
	{
		ClearCommError(m_CommPort, &dwErrorFlags, &ComStat);
		fReadStat = ReadFile(m_CommPort, pszBuf+i, RecvCnt-i,
			&dwReaded, NULL);

		if (!fReadStat)
		{
			dwError = GetLastError();
			// some other error occurred
			ClearCommError(m_CommPort, &dwErrorFlags, &ComStat);
			return -1;
		}

		if(dwReaded ==0)
		{
			Count=GetTickCount();
			if(Count-OldCount>=(unsigned long)TimeOut) //修改:timeout为int型 ，强制转换为UL
				return -2;
			Sleep(10);
		}
		else
		{
			i += dwReaded;	
			if (i >= (unsigned long)RecvCnt)//修改:timeout为int型 ，强制转换为UL
				return 0;
		}
	} 
}


void SaveRecv(BYTE *str, int iLen)
{
#ifdef _COMM_DEBUG_
	FILE *fp;

	fp = fopen("RecvData.bin", "rb+");
	if (NULL == fp)
	{
		fp = fopen("RecvData.bin", "wb+");
		if (NULL == fp)
		{
			return ;
		}
	}
	fseek(fp, 0, SEEK_END);
	fwrite(str, sizeof(BYTE), iLen, fp);
	fclose(fp);
#endif
}



void SaveSend(BYTE *str, int iLen)
{
#ifdef _COMM_DEBUG_
	FILE *fp;

	fp = fopen("SendData.bin", "rb+");
	if (NULL == fp)
	{
		fp = fopen("SendData.bin", "wb+");
		if (NULL == fp)
		{
			return ;
		}
	}
	fseek(fp, 0, SEEK_END);
	fwrite(str, sizeof(BYTE), iLen, fp);
	fclose(fp);
#endif
}
