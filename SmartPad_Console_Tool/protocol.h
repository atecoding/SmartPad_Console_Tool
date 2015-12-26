#pragma once
#include "port.h"

class SmartProtocol
{
private:
	SmartPort port;
	int g_percent;
	int protocol_send( unsigned char *cmd, unsigned long lg, unsigned short timeout);
	int protocol_receive( unsigned char *rsp, unsigned long *lg, unsigned short timeout);
	int protocol_transfer(unsigned char *send_buf, unsigned long lg, unsigned char* rec_buf, unsigned long* rec_len, unsigned int timeout);
	int protocol_cmd(unsigned char* i_cmdsend, unsigned int i_cmdlen, unsigned char* o_cmdreply, unsigned int* o_cmdreplylen, unsigned int i_trytime, unsigned int timeout);
	int protocl_send_FileHead(unsigned char* filename,unsigned char Category, int filesize);
	int protocol_send_FileFrame(unsigned char* filedata, unsigned int filedatasize, int frameNo, int frameRemain);
	int protocol_sendfile(unsigned char* filepath, unsigned char Category);
	int protocol_update_files(unsigned char* filename,unsigned char ucCategory);
	int get_version(char *version, unsigned char category);
public:
	SmartProtocol(SmartPort &p);
	virtual ~SmartProtocol(void);
public:
	int get_ipk_version(char *version);
	int get_config_version(char *version);
	int update_config(char *config_path);
	int update_ipk(char *ipk_path);
	int reboot();
};



