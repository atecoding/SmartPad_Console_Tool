//#include "stdafx.h"
#include "ComPortVidQuery.h"
#include <tchar.h>
#include <setupapi.h>
//#include <string>
#include "iostream"
using namespace std;

#pragma comment (lib, "Setupapi.lib")

#define DeviceInstanceIdSize 256	// 设备实例ID最大长度

// 获取系统的VID和PID集合
string WINAPI WDK_WhoAllVidPid(unsigned int pid, unsigned int vid)
{
	
	HIDD_VIDPID* pVidPid=new HIDD_VIDPID[9];
	int iCapacity=32;
	GUID* SetupClassGuid=NULL;
	GUID* InterfaceClassGuid=NULL;
	BYTE fname[128] = {0};
	string COM(8,0);
	
	// 根据设备安装类GUID创建空的设备信息集合
	HDEVINFO DeviceInfoSet = SetupDiCreateDeviceInfoList( SetupClassGuid, NULL );
	if (DeviceInfoSet == INVALID_HANDLE_VALUE) return COM;
	
	// 根据设备安装类GUID获取设备信息集合
	HDEVINFO hDevInfo;
	
	if(InterfaceClassGuid == NULL)
		
		hDevInfo = SetupDiGetClassDevsEx( NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE | DIGCF_PRESENT, DeviceInfoSet, NULL, NULL );
	else
		hDevInfo = SetupDiGetClassDevsEx( InterfaceClassGuid, (PCSTR)&"SCSI", NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT, DeviceInfoSet, NULL, NULL );

	if (hDevInfo == INVALID_HANDLE_VALUE) return COM;


	
	// 存储设备实例ID	
	TCHAR DeviceInstanceId[DeviceInstanceIdSize];	

	// 存储设备信息数据
	SP_DEVINFO_DATA DeviceInfoData;					
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	// 获取设备信息数据
	DWORD DeviceIndex = 0;
	 bool k=true;
	while (SetupDiEnumDeviceInfo( hDevInfo, DeviceIndex++, &DeviceInfoData))
	{
				
		// 获取设备实例ID
		if (SetupDiGetDeviceInstanceId(hDevInfo, &DeviceInfoData, DeviceInstanceId, DeviceInstanceIdSize, NULL)&&k)
		{
			// 从设备实例ID中提取VID和PID
			TCHAR* pVidIndex = _tcsstr(DeviceInstanceId, TEXT("VID_"));
			if (pVidIndex == NULL) continue;

			TCHAR* pPidIndex = _tcsstr(pVidIndex + 4, TEXT("PID_"));
			if (pPidIndex == NULL) continue;

			USHORT VendorID = (USHORT)_tcstoul(pVidIndex + 4, NULL, 16);
			USHORT ProductID = (USHORT)_tcstoul(pPidIndex + 4, NULL, 16);

			if(VendorID != vid && ProductID != pid) {
				continue;
			}
			
			//	输出串口号
			SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData,
				SPDRP_FRIENDLYNAME , 
				0,(PBYTE) fname,
				sizeof(fname),
				NULL);

			string str2(57, '0');
			string s2="COM";
			for (int i=0;i<56;i++)
			{
				str2[i]=(char)fname[i];
			}

			if (str2.find(s2) != string::npos)
			{
				int num = str2.find(s2);
				COM  = str2.substr(num,5);
			}			
		}
	}
	return COM;
}

