#include "Common.h"


static char ERROR_MSG[][128] = {
	"<common error,-1>",
	"-100",
	"<-101>",
	"<-102>",
	"<-103>",
	"<Firmware no not support,-104>",
	"<-105>",
	"<-106>",
	"<-107>",
	"<-108>",
};

char * get_error_msg(int errorno) {
	int e = 0;
	if (errorno > 0){
		return "<unkown>";
	} else if(errorno == -1){
		e = CONSOLE_ERROR_BASE;
	} else {
		e =  CONSOLE_ERROR_BASE - errorno ;
	}
	if(e<0 || e> sizeof(ERROR_MSG)/128){
		return "<unkown>";
	}
	return ERROR_MSG[e];
}