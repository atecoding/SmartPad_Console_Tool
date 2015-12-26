#pragma once
#include <windows.h>

#define MAX_BUFFER_IN     (8192)
#define MAX_BUFFER_OUT    (8192)

#define ERR_PORTID			-100
#define ERR_QUEUE			-101
#define ERR_COMMSET			-102
#define ERR_SET_TIMEOUT		-103
#define ERR_RECV_TO			-104
#define ERR_SEND_TO			-105
#define ERR_SEND_BYTE		-106
#define ERR_RECV_BYTE		-107
#define ERR_RECV_LEN		-108
#define	ERR_NOTOPEN			-109
//#define ERR_NOTOPEN			-110
#define ERR_NOTCLOSE		-111
#define ERR_RECV_TIMEOUT    -112

class CSeries
{
public:
	CSeries(void);
	virtual ~CSeries(void);
public:
	/*�򿪴���*/
public:
	HANDLE m_CommPort;
public:
	/*��ս��ջ�����*/
	void  PurgeIn();
	/*��շ��ͻ�����*/
	void  PurgeOut();
	/*��ѯ���ջ�����*/
	int  InQueCount();
	/*��ѯ���ͻ�����*/
	int  OutQueCount();
	/*�رյ�ǰ����*/
	int SerReset(void);
	int SerClose();
	int SerOpen(int PortNo, char * param);
	int SerRead(BYTE *pszBuf , int RecvCnt , int TimeOut);
	int SerWrite(BYTE *pszBuf , int SendCnt , int TimeOut);


};

