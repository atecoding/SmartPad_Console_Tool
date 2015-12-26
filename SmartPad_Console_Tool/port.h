#pragma once
#include "Cserial.h"

typedef enum 
{
	USB,
	RS232,
}SmartPortType;

class SmartPort
{
private:
	SmartPortType portType;
	CSeries serial;
public:
	SmartPort(SmartPortType t, int port_NUM, char *param);
	virtual ~SmartPort(void);
public:
	/*清空接收缓冲区*/
	void  PurgeIn();
	/*清空发送缓冲区*/
	void  PurgeOut();
	/*查询接收缓冲区*/
	int  InQueCount();
	/*查询发送缓冲区*/
	int OutQueCount();

	int Reset(void);
	int Close();
	int Open(int port_NUM, char *param);
	int Read(unsigned char *pszBuf , int RecvCnt , int TimeOut);
	int Write(unsigned char *pszBuf , int SendCnt , int TimeOut);
	int Transfer(unsigned char *send_buf, int send_size, int send_timeout, unsigned char *rec_buf, int *rec_size, int rec_timeout);
};
