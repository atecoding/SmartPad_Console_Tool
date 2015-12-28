/* Smartpad console tool */
/* Create by colin in Dec,20,2015 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <stdio.h>

#include "ComPortVidQuery.h"
#include "Common.h"
#include "port.h"
#include "protocol.h"
#include "padUsb.h"

#define VERSION "v_0.0.2_20151228"

int main(int argc, char **argv) 
{
	string com, comNUM;
	int iComN = 0, i = 1;
	SmartPortType commType = RS232;
	unsigned char buf_out[1024 + 1] = {0};

	char buf_in[512 + 1] = {0};
	SmartPort * SP = NULL;
	int flow = 0, ret = 0, if_query_usb = 0;

	while(i < argc) {
		if( !strcmp(argv[i], "usb+") ) {
			if_query_usb = 1;
		} else if( !strcmp(argv[i], "check_firmware") && flow == 0){
			flow = 1;
		} else if( !strcmp(argv[i], "config") && flow == 0){
			flow = 2;
			if( (i+i < argc) && (argv[i+1] != NULL) && (argv[i+1][0] != 0x00) ) {
				strcpy(buf_in, argv[i+1]);
				if (!if_file_exit(buf_in)) {
					cout << "01_<Can not find the file:" <<buf_in <<">";
					return CONSOLE_ERROR_FILE_CAN_NOT_FOUND;
				}
			} else {
				flow = 0xFFFF;
			}
		}
		else if( !strcmp(argv[i], "reboot") && flow == 0){
			flow = 3;
		}
		else if( !strcmp(argv[i], "firmware") && flow == 0){
			flow = 4;
			if( (i+i < argc) && (argv[i+1] != NULL) && (argv[i+1][0] != 0x00) ) {
				strcpy(buf_in, argv[i+1]);
				if (!if_file_exit(buf_in)) {
					cout << "01_<Can not find " <<buf_in <<">";
					return CONSOLE_ERROR_FILE_CAN_NOT_FOUND;
				}
			} else {
				flow = 0xFFFF;
			}
		}
		else if( !strcmp(argv[i], "check_config") && flow == 0){
			flow = 5;
		} else if( !strcmp(argv[i], "version") && flow == 0){
			flow = 6;
		}
		i++;
	}


	if (flow == 0) {
		cout << "01_<No valid parameter>";
		return CONSOLE_ERROR_CANNOT_NO_VALIED_PARAM;
	} else 	if (flow == 0xFFFF) {
		cout << "01_<Parameter error>";
		return CONSOLE_ERROR_CANNOT_PARAM_ERROR;
	} else if (flow == 6) {
		std::cout << VERSION;
		return 0;
	}
	else {
	
		com = WDK_WhoAllVidPid(SMARTPAD_PID, SMARTPAD_VID);

		if (com != "" && com[0] != 0) {
			//cout << "Smartpad is using " << com << endl;
			comNUM = com.substr(3, com.length() - 3);
			iComN = atoi(comNUM.data());
			commType = RS232;
			SP = new SmartPort(commType, iComN, "baud=115200 data=8 parity=N stop=1 dtr=on rts=on");
		} else if(if_usb(if_query_usb)){
			commType = USB;
			SP = new SmartPort(commType, 0, NULL);
		} else {
			cout << "01_Can not connect to the SmartPad, Please connect the Smartpad to PC first";
			return CONSOLE_ERROR_CANNOT_FIND_SMARTPAD;
		}

		if (SP == NULL) {
			cout << "01_Connect to the SmartPad Error";
			return CONSOLE_ERROR_CONNECTON_ERROR;
		}
	}

	

	SmartProtocol protocol(*SP);

	switch (flow) {
	case 1: 
		ret = protocol.get_ipk_version((char *)buf_out);
		if( ret == 0) {
			std:: cout << "00_" << buf_out;
		}
		break;
	case 2:
		ret = protocol.update_config(buf_in);
		if (ret == 0) {
			cout << "00_Update successfully";
		}
		break;
	case 3:
		ret = protocol.reboot();
		break;
	case 4:
		ret = protocol.update_ipk(buf_in);
		if (ret == 0) {
			cout << "00_Update successfully";
			protocol.reboot();
		}
		break;
	case 5:
		ret = protocol.get_config_version((char *)buf_out);
		if (ret == 0) {
			std:: cout << "00_" << buf_out;
		}
		break;
	}

	if (SP != NULL) {
		delete SP;
	}

exit:
	if (ret != 0) {
		std:: cout << "01_"<< get_error_msg(ret);
		return ret;
	} else {
		//cout  << "successfully";
	}
	
	return 0;
}

