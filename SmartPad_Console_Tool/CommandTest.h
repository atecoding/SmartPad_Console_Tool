#pragma once
#include "protocol.h"
int command_print(const char *format, ...);
int command_test(SmartProtocol &p, char **argv);