#include "Common.h"
#include <stdio.h>

static char ERROR_MSG[][128] = {
	"<Common error,-1>",
	"-100",
	"<-101>",
	"<-102>",
	"<-103>",
	"<Firmware do no not support,-104>",
	"<-105>",
	"<-106>",
	"<-107>",
	"<-108>",
};

char * get_error_msg(int errorno) {
	int e = 0;
	if (errorno > 0){
		return "<Unknown,1>";
	} else if(errorno == -1){
		e = CONSOLE_ERROR_BASE;
	} else {
		e =  CONSOLE_ERROR_BASE - errorno ;
	}
	if(e<0 || e> sizeof(ERROR_MSG)/128){
		return "<Unknown,-1>";
	}
	return ERROR_MSG[e];
}

int if_file_exit(const char *path) {
	FILE *f = NULL;
	f = fopen(path, "r");
	if (f == NULL) {
		return 0;
	} else {
		fclose(f);
		return 1;
	}
}