#include "CommandTest.h"
#include <iostream>

char s[1024] = {0};
SmartProtocol *pp;

int command_print(const char *format, ...)
{
	va_list args;
	va_start(args, format);	
	vsnprintf(s, sizeof(s), format, args);
	va_end(args);
	pp->print(s, strlen(s));
	return 0;
}

extern int complie_file(int argc, char **argv);

int command_test(SmartProtocol &p, char **argv)
{
	*pp = p;
	if (strcmp(argv[0], "-f") == 0) {
		complie_file(3, argv);
	}
	return 0;
}
