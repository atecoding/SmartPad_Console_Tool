/* ----------------------------------------------------------
�ļ����ƣ�WDK_VidPidQuery.h

����������
	Visual Studio V2008 

�ӿں�����
	WDK_WhoAllVidPid
------------------------------------------------------------ */
#pragma once
#include <string>
using namespace std;
#include <windows.h>


#ifndef MACRO_HIDD_VIDPID
	#define MACRO_HIDD_VIDPID
	typedef struct _HIDD_VIDPID
	{
		USHORT	VendorID;
		USHORT	ProductID;
	} HIDD_VIDPID;
#endif


/*
���ܣ���ȡ��ӦVID�豸�Ĵ��ں�
��ڲ�����
	[in] ssin���豸��VID��
����ֵ��
	��ȡ���Ĵ��ں�
*/
	string  WINAPI WDK_WhoAllVidPid(unsigned int pid, unsigned int vid);


/*
vid of smartpad: %d 1947
pid 
*/