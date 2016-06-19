#include "CommandTest.h"
#include <iostream>

char r[1024] = {0};
int r_len = 0;
SmartProtocol *pp;

SmartProtocol * get_p(void)
{
	return pp;
}

int str_cp(char *in, char *out)
{
	while (*in) {
		if (*in != 0x20) {
			*out = *in;
			out++;
			in++;
		} else {
			in++;
		}
	}
	return 0;
}

int str_to_hex(char *in, char *out)
{
	unsigned int i = 0;
	for (i = 0; i < (strlen(in) / 2); i++) {
         sscanf(in+i*2, "%02hhx", out+i);
    }
	return i;
}

int command_print(const char *format, ...)
{
	char s[1024] = {0};
	char ss[1024] = {0};
	int  hx_l;
	va_list args;
	va_start(args, format);	
	vsnprintf(s, sizeof(s), format, args);
	va_end(args);

	memset(r, 0, sizeof r);
	str_cp(s, ss);
	std::cout << "Command:" << ss << std::endl;
	hx_l = str_to_hex(ss, s);

	(SmartProtocol *)get_p()->print(s, hx_l, r, &r_len);
	std::cout << "Response:" << r << std::endl;
	return 0;
}

extern int complie_file(int argc, char **argv);

int command_test(SmartProtocol *p, char **argv)
{
	pp = p;
	if (strcmp(argv[0], "-c") == 0) {
		if (argv[1] != NULL && argv[1][0] != 0)
			complie_file(2, argv);
	}
	return 0;
}
