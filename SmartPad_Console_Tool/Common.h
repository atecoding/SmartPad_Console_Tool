#pragma once

#define SMARTPAD_VID 0x079B
#define SMARTPAD_PID 0x05A0

#define CONSOLE_ERROR_BASE                   -99
#define CONSOLE_ERROR_CANNOT_FIND_SMARTPAD	 -100
#define CONSOLE_ERROR_CONNECTON_ERROR		 -101
#define CONSOLE_ERROR_CANNOT_PARAM_ERROR	 -102
#define CONSOLE_ERROR_CANNOT_NO_VALIED_PARAM -103
#define CONSOLE_ERROR_FIRMWARE_DO_NOT_SUPPOR -104
#define CONSOLE_ERROR_FILE_CAN_NOT_FOUND	 -105

char * get_error_msg(int errorno);
int if_file_exit(const char *path);