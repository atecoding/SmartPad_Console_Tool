/* ----------------------------------------------------------
文件名称：WDK_VidPidQuery.h

开发环境：
	Visual Studio V2008 

接口函数：
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
功能：获取对应VID设备的串口号
入口参数：
	[in] ssin：设备的VID号
返回值：
	获取到的串口号
*/
	string  WINAPI WDK_WhoAllVidPid(unsigned int pid, unsigned int vid);


/*
vid of smartpad: %d 1947
pid 
*/