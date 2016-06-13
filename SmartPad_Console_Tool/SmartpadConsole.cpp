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
#include "command.h"
#include "version.h"
#include "CommandTest.h"

static int if_query_usb = 0;

static int help(char *which)
{


	return 0;
}

static int parse_cmd(int argc, char **argv, char *out, int out_len, char ***argv_out)
{
	int i = 0;
	int cmd  = 0;
	char buf_in[512] = {0};

	while(i < argc)
	{
		if ( !strcmp(argv[i], "usb+") ) 
		{
			if_query_usb = 1;
		} 
		else if ( !strcmp(argv[i], "check_firmware") && cmd == CMD_VALID)
		{
			cmd = CMD_CHECK_FIRMWARE_VERSION;
		}
		else if ( !strcmp(argv[i], "config") && cmd == CMD_VALID)
		{
			cmd = CMD_CHECK_CONFIG_VERSION;
			if ( (i + i < argc) && (argv[i + 1] != NULL) && (argv[i + 1][0] != 0x00) ) 
			{
				strcpy(buf_in, argv[i + 1]);
				if (!if_file_exit(buf_in))
				{
					std::cout << "01_<Can not find the file:" <<buf_in <<">";
					return CONSOLE_ERROR_FILE_CAN_NOT_FOUND;
				}
			} 
			else
			{
				cmd = CMD_ERROR;
			}
		}
		else if ( !strcmp(argv[i], "reboot") && cmd == CMD_VALID)
		{
			cmd = CMD_REBOOT;
		}
		else if( !strcmp(argv[i], "firmware") && cmd == CMD_VALID)
		{
			cmd = CMD_UPDATE_FIRMWARE;
			if ( (i + 1 < argc) && (argv[i + 1] != NULL) && (argv[i + 1][0] != 0x00) )
			{
				strcpy(buf_in, argv[i + 1]);
				if (!if_file_exit(buf_in)) 
				{
					std::cout << "01_<Can not find " <<buf_in <<">";
					return CONSOLE_ERROR_FILE_CAN_NOT_FOUND;
				}
			} 
			else 
			{
				cmd = CMD_ERROR;
			}
		}
		else if ( !strcmp(argv[i], "check_config") && cmd == CMD_VALID)
		{
			cmd = CMD_UPDATE_CONFIG;
		} 
		else if ( !strcmp(argv[i], "version") && cmd == CMD_VALID)
		{
			cmd = CMD_GET_TOOL_VERSION;
		} 
		else if (!strcmp(argv[i], "test") && cmd == CMD_VALID)
		{
			cmd = CMD_GET_TOOL_HELP; 
			if( (i + 1 < argc) && (argv[i + 1] != NULL) && (argv[i + 1][0] != 0x00) )
			{
				strcpy(buf_in, argv[i + 1]);
			} 
		}
		else if (!strcmp(argv[i], "test") && cmd == CMD_VALID) 
		{
			cmd = CMD_TEST;
			if( (i + 1 < argc) && (argv[i + 1] != NULL) && (argv[i + 1][0] != 0x00) )
			{
				*argv_out = &argv[i + i];
			} 
			else 
			{
				cmd = CMD_ERROR;
			}		
		}
		i++;
	}
	vsnprintf(out, out_len, "%s", buf_in);
	return cmd;

}

int main(int argc, char **argv) 
{
	int iComN = 0, i = 1;
	unsigned char buf_out[1024 + 1] = {0};
	char buf_in[512 + 1] = {0};
	int flow = CMD_VALID, ret = 0;
	SmartPort * SP = NULL;
	char **argv2 = NULL;

	flow = parse_cmd(argc, argv, buf_in, sizeof buf_in, &argv2);
	
	if (flow == CMD_VALID) 
	{
		std::cout << "01_<No valid parameter>";
		return CONSOLE_ERROR_CANNOT_NO_VALIED_PARAM;
	} 
	else if (flow == CMD_ERROR) 
	{
		std::cout << "01_<Parameter error>";
		return CONSOLE_ERROR_CANNOT_PARAM_ERROR;
	} 
	else if (flow == CMD_GET_TOOL_VERSION) 
	{
		std::cout << APP_VERSION;
		return CONSOLE_TOOL_SUCCUSS;
	} 
	else if (flow == CMD_GET_TOOL_HELP)
	{
		return help(buf_in);
	}
	else 
	{
		std::string com = WDK_WhoAllVidPid(SMARTPAD_PID, SMARTPAD_VID);
		SmartPortType commType = RS232;
		if (com != "" && com[0] != 0) 
		{
			//cout << "Smartpad is using " << com << endl;
			std::string comNUM = com.substr(3, com.length() - 3);
			iComN = atoi(comNUM.data());
			commType = RS232;
			SP = new SmartPort(commType, iComN, "baud=115200 data=8 parity=N stop=1 dtr=on rts=on");
		} 
		else if(if_usb(if_query_usb))
		{
			commType = USB;
			SP = new SmartPort(commType, 0, NULL);
		} 
		else
		{
			std::cout << "01_Can not connect to the SmartPad, Please connect the Smartpad to PC first";
			return CONSOLE_ERROR_CANNOT_FIND_SMARTPAD;
		}

		if (SP == NULL) {
			std::cout << "01_Connect to the SmartPad Error";
			return CONSOLE_ERROR_CONNECTON_ERROR;
		}
	}

	
	SmartProtocol protocol(*SP);

	switch (flow) {
	case CMD_CHECK_FIRMWARE_VERSION: 
		ret = protocol.get_ipk_version((char *)buf_out);
		if( ret == 0) {
			std:: cout << "00_" << buf_out;
		}
		break;
	case CMD_UPDATE_CONFIG:
		ret = protocol.update_config(buf_in);
		if (ret == 0) {
			std::cout << "00_Update config file successfully";
		}
		break;
	case CMD_REBOOT:
		ret = protocol.reboot();
		//cout << ret;
		break;
	case CMD_UPDATE_FIRMWARE:
		ret = protocol.update_ipk(buf_in);
		if (ret == 0) {
			std::cout << "00_Update firmware successfully";
			protocol.reboot();
		}
		break;
	case CMD_CHECK_CONFIG_VERSION:
		ret = protocol.get_config_version((char *)buf_out);
		if (ret == 0) {
			std:: cout << "00_" << buf_out;
		}
		break;
	case CMD_TEST:
		std::cout << "after test:" << argv2[0] << std::endl;
		return command_test(protocol, argv2);
	}

	if (SP != NULL) {
		delete SP;
	}

	if (ret != 0) {
		std:: cout << "01_"<< get_error_msg(ret);
		return ret;
	} else {
		//cout  << "successfully";
	}
	
	return CONSOLE_TOOL_SUCCUSS;
}

