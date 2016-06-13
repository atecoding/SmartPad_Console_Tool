#include <stdio.h>
#include "string.h"
#include "protocol.h"
#include "port.h"
#include "Common.h"
#include <iostream>

#define STX_LEN  (4)
#define ETX_LEN  (2)
#define DEFAULT_TMO (200)
#define LOOP_DELAY  (3)
#define SENDFILE_FRAME_MAXDATA (1024)
#define SIG_FNAME_MAX      16

//function return defination
#define SIGLIB_RET_SUCCESS 0         //Function success
#define SIGLIB_RET_FAILED  1         //No respond from Pad, or Pad not connected
#define SIGLIB_RET_TIMEOUT 0xe0      //Timeout when sign or select DCC flag
#define SIGLIB_RET_BADPARAMETER 0xe1 //Input parameter wrong
#define SIGLIB_RET_RESPONDERROR 0xe2 //Respond from Pad is wrong
#define SIGLIB_RET_NOSIGNCAPTURE  0xe3 //No signature but press confirm button
#define SIGLIB_RET_FILEERROR    0xe4  //for TTA cannot get file error
#define SIGLIB_RET_UNKNOWNERROR 0xef //POS internal error
#define SIGLIB_RET_FILEEUPDATEERR  0xF0

//function parameter defination
#define SIGLIB_PARA_MODELOCK 0       //For siglib_SetLockMode use.
#define SIGLIB_PARA_MODEUNLOCK 1     //For siglib_SetLockMode use.
#define SIGLIB_PARA_SIGNATURE 0      //For siglib_SetSmartPadMode use.
#define SIGLIB_PARA_PINPAD    1      //For siglib_SetSmartPadMode use.
#define SIGLIB_PARA_DISPLAY 1        //For siglib_Signature use.
#define SIGLIB_PARA_NODISPLAY 0      //For siglib_Signature use.

#define SIGLIB_FILE_CATAGORY_IPK				0x02
#define SIGLIB_FILE_CATAGORY_BUS_CONFIG			0x0C

unsigned char stx[STX_LEN] = {0x53,0x59,0x4E,0x43}; //SYNC
unsigned char etx[ETX_LEN] = {0x45,0x4E} ; //EN

SmartProtocol::SmartProtocol(SmartPort &p) :port(p)  {
	return;
}

SmartProtocol::~SmartProtocol(void)  {
	return;
}


int SmartProtocol::protocol_transfer(unsigned char *send_buf, unsigned long lg, unsigned char* rec_buf, unsigned long* rec_len, unsigned int timeout){
	int ii = 0, ret = 0;
	unsigned char buf[8192];
	unsigned long nlg;
	unsigned char buffer[512] = {0};
	int reclen = 512;
	unsigned long ulen,unlen;
	int i =0;
	
	memcpy(buf,stx,STX_LEN);

	nlg = 0xffffffff^lg;
	memcpy(buf+STX_LEN,&lg,sizeof(unsigned long));
	memcpy(buf+STX_LEN+sizeof(unsigned long),&nlg,sizeof(unsigned long));
	memcpy(buf+STX_LEN+8,send_buf,lg);
	memcpy(buf+STX_LEN+8+lg,etx,ETX_LEN);
/*
	printf("UsbWrite: ");
	for(i=0; i<lg+14; i++){
		printf("%02x ", (unsigned char)buf[i]);
	}
	printf("\n");
*/
	ret = port.Transfer(buf, STX_LEN+8+lg+2, 500, buffer, &reclen, timeout);

	if (ret < 0)
	{
		//TRACE("error1[%d]\r\n",iret);
		return -1;
	}

	if(memcmp(buffer,stx,STX_LEN))
	{
		//TRACE("error6[%d]\r\n",iret);
		return -1 ; 
	}
	ulen = MAKELONG( MAKEWORD(buffer[0+4],buffer[1+4]),MAKEWORD(buffer[2+4],buffer[3+4]));
	unlen = MAKELONG( MAKEWORD(buffer[4+4],buffer[5+4]),MAKEWORD(buffer[6+4],buffer[7+4]));
//	memcpy(&ulen, buffer+4, 4);
	//memcpy(&unlen, buffer+8, 4);


	if ((ulen^0xffffffff) != unlen)
	{
		//TRACE("ulen[%d] unlen[%d]\r\n",ulen,unlen);
		return -4;
	}

	if(memcmp(buffer+4*3+ulen,etx,ETX_LEN))
	{
		//TRACE("error6[%d]\r\n",iret);
		return -7 ; 
	}
	memcpy(rec_buf,buffer+4*3,ulen);
	*rec_len = ulen ; 
/*
	printf("UsbRead: ");
	for(i=0; i<14+(int)ulen; i++){
		printf("%02x ", (unsigned char)buffer[i]);
	}
	printf("\n");
*/
	return 0;
	
}

int SmartProtocol::protocol_send( unsigned char *cmd,  unsigned long  lg, unsigned short timeout)
{
	unsigned char buf[8192];
	unsigned long nlg;
	memcpy(buf,stx,STX_LEN);
	
	nlg = 0xffffffff^lg;
	memcpy(buf+STX_LEN,&lg,sizeof(unsigned long));
	memcpy(buf+STX_LEN+sizeof(unsigned long),&nlg,sizeof(unsigned long));
	memcpy(buf+STX_LEN+8, cmd,lg);
	memcpy(buf+STX_LEN+8+lg,etx,ETX_LEN);

	port.Reset();
	port.Write(buf, STX_LEN + 8 + lg + 2, timeout);
	
	//SerWrite(stx,STX_LEN,tmo);
	//SerWrite(ilv_cmd,lg,DEFAULT_TMO);
	//SerWrite(etx,ETX_LEN,DEFAULT_TMO);
	/*TRACE("%s: ",__FUNCTION__);
	for(int i =0;i<STX_LEN+8+lg+2;i++)
		TRACE("%02X ",buf[i]);
	TRACE("\r\n");*/
	
	return 0;
}

int SmartProtocol::protocol_receive( unsigned char *rsp,  unsigned long  *lg, unsigned short timeout) {
	BYTE buffer[1024];
	unsigned long ulen,unlen;
	int iret ,i; 

	ZeroMemory(buffer,sizeof(buffer));
	//TRACE("%s: ",__FUNCTION__);
	iret = port.Read(buffer, STX_LEN, timeout);
	if (iret)
	{
		//TRACE("error1[%d]\r\n",iret);
		return -1;
	}

	//for(i =0;i<STX_LEN;i++)
	//	TRACE("%02X ",buffer[i]);
	if(memcmp(buffer,stx,STX_LEN))
	{
		//TRACE("error2[%d]\r\n",iret);
		return -2 ; 
	}

	iret =	port.Read(buffer, 8 ,timeout);
	if (iret)
	{
		//TRACE("error3[%d]\r\n",iret);
		return -3;
	}
	for(i =0;i<8;i++)
		//TRACE("%02X ",buffer[i]);

    ulen = MAKELONG( MAKEWORD(buffer[0],buffer[1]),MAKEWORD(buffer[2],buffer[3]));
	unlen = MAKELONG( MAKEWORD(buffer[4],buffer[5]),MAKEWORD(buffer[6],buffer[7]));

	if ((ulen^0xffffffff) != unlen)
	{
		//TRACE("ulen[%d] unlen[%d]\r\n",ulen,unlen);
		return -4;
	}

	iret = port.Read(buffer, ulen+ETX_LEN, DEFAULT_TMO);
	if (iret)
	{
		//TRACE("error4[%d]\r\n",iret);
		return -5;
	}
	//for(i =0;i<ulen+ETX_LEN;i++)
	//	TRACE("%02X ",buffer[i]);

	if(memcmp(buffer+ulen,etx,ETX_LEN))
	{
		//TRACE("error6[%d]\r\n",iret);
		return -7 ; 
	}
	//TRACE("ulen=%d\r\n",ulen);
	memcpy(rsp, buffer, ulen);
	*lg = ulen ; 
	

	//g_pad_port.SerReset();
	port.Reset();

	return 0 ;
}

int SmartProtocol::protocol_cmd(unsigned char* i_cmdsend, unsigned int i_cmdlen, unsigned char* o_cmdreply, unsigned int* o_cmdreplylen, unsigned int i_trytime, unsigned int timeout)
{
	unsigned long m_len = 0;
	int m_ret;
	unsigned int i_cmdcount = 0;
	unsigned char str[1024] = {0};
	unsigned char *ppstr =(unsigned char * )str;
	int loop_time = 0;

	//memcpy(ppstr,"\xff\xff\xff\xaa\xaa\xaa",6);
	i_cmdcount = i_trytime;
	memset(str, 0 , sizeof(str));
	while((i_cmdcount --) || (i_trytime == 0))
	{
		while((loop_time ++ ) <= (LOOP_DELAY / 3))
		{
			//m_ret = protocol_send(i_cmdsend, i_cmdlen, 1000);//BOTH
			//SaveSend(i_cmdsend,i_cmdlen);
			//TRACE(TEXT("Morpho_SendUserILV\r\n"));
			m_len = 0;
			
			//m_ret = protocol_receive(ppstr, &m_len, timeout);
			//TRACE(TEXT("Morpho_ReadUserILV m_ret=%d\r\n"),m_ret);
			m_ret = protocol_transfer((unsigned char *)i_cmdsend, i_cmdlen, ppstr, &m_len, timeout);
			//printf("%d\n", m_ret);
			if(m_ret == 0 && m_len >0)
			{
				//SaveRecv(str,m_len);
				break;
			}
		}
		//printf("%x\n", ppstr[0]);
		//printf("%02x %02x %02x \n", ppstr[3], ppstr[0], i_cmdsend[0]);
		if(m_len > 0 && ppstr[3] == 0 && ppstr[0] == i_cmdsend[0])
		{
			memcpy(o_cmdreply, ppstr, m_len);
			m_ret = SIGLIB_RET_SUCCESS; //success
			*o_cmdreplylen = m_len;
			break;
		}
		else
		{

			m_ret = SIGLIB_RET_RESPONDERROR; //no respond and time out
		}
	}
	//printf("protocol_cmd ret=%d\n", m_ret);
	return m_ret;
}

int SmartProtocol::get_version(char *version, unsigned char category) {
	int m_ret;
	unsigned char m_ret_data[1024];
	unsigned char m_senddata[64];
	unsigned int m_ret_len;
	BYTE Status;
//	unsigned int mcount;
	ZeroMemory(m_ret_data,sizeof(m_ret_data));
	ZeroMemory(m_senddata,sizeof(m_senddata));
	
	//if(ucCategory ==KER_VER_ID )
	memcpy(m_senddata, "\x39\x01\x00", 3);
	m_senddata[3] = category;

	m_ret = protocol_cmd((unsigned char*)m_senddata, 4, m_ret_data, &m_ret_len, 1, 1000);
	
	if(m_ret != SIGLIB_RET_SUCCESS)
			return m_ret;
	if(m_ret_data[0] != 0x39)
	{
		m_ret = SIGLIB_RET_RESPONDERROR;
		return m_ret;
	}
	Status = m_ret_data[3];
	if (Status)
	{
		memcpy(version, m_ret_data+4, 16);
		//TRACE(TEXT("%s\r\n"),VerInfo);
	}
	else
	{
		memcpy(version, m_ret_data+4, 16);
		//TRACE(TEXT("%s\r\n"),VerInfo);
	}

	return m_ret;
}

int SmartProtocol::get_ipk_version(char *version) {
	return get_version(version, 0x01);
}

int SmartProtocol::get_config_version(char *version) {
	return get_version(version, 0x02);
}

int SmartProtocol::reboot() {

	unsigned char m_ret;
	unsigned char m_ret_data[1024];
	unsigned int m_ret_len = 0;

	memset(m_ret_data, 0x00, sizeof(m_ret_data));	
	m_ret = protocol_cmd((unsigned char*)"\x92\x00\x00", 3, m_ret_data, &m_ret_len, 1, 500);
	if(m_ret != SIGLIB_RET_SUCCESS)
	{
		return m_ret;
	}
	if(m_ret_data[0] != 0x92)
		m_ret = SIGLIB_RET_RESPONDERROR;
	return m_ret;
}

int SmartProtocol::protocl_send_FileHead(unsigned char* filename,unsigned char Category, int filesize)
{
	unsigned char m_ret;
	unsigned char m_ret_data[1024];
	unsigned char m_senddata[64];
	unsigned int m_ret_len;
	unsigned int mcount;
	memset(m_ret_data, 0x00, sizeof(m_ret_data));
	memset(m_senddata, 0x00, sizeof(m_senddata));
	mcount = 0;
	m_senddata[mcount++] = 0x35; //command Id
	m_senddata[mcount++] = 0x00;
	m_senddata[mcount++] = 0x00;   //The total length of the command and need to modify at final!
	m_senddata[mcount++] = Category; 

	memcpy(m_senddata + mcount, filename, SIG_FNAME_MAX); //filename, 16 bytes
	mcount = mcount+16;

	m_senddata[mcount++] = filesize & 0x000000ff;  //file size, 4 bytes
	m_senddata[mcount++] = (filesize>>8) & 0x000000ff;
	m_senddata[mcount++] = (filesize>>16) & 0x000000ff;
	m_senddata[mcount++] = (filesize>>24) & 0x000000ff;

	//put full data length
	m_senddata[1] = (mcount-3) & 0x000000ff;
	m_senddata[2] = ((mcount-3)>>8) & 0x000000ff;

	m_ret = protocol_cmd(m_senddata, mcount, m_ret_data, &m_ret_len, 1, 1000);
	if(m_ret != SIGLIB_RET_SUCCESS)
		return m_ret;
	if(m_ret_data[0] != 0x35)
		m_ret = SIGLIB_RET_RESPONDERROR;
	return m_ret;
}


int fit_percent(int percent ,int total)
{
	int temp;
	temp=percent;
	temp = (percent*1000)/total;
	temp/=10;
	if (temp == total)
	{
		temp =100;
	}
	return temp;

}

void thisSaveSend(BYTE *str, int iLen)
{
#ifdef _COMM_DEBUG_
	FILE *fp;

	fp = fopen("SendData.bin", "rb+");
	if (NULL == fp)
	{
		fp = fopen("SendData.bin", "wb+");
		if (NULL == fp)
		{
			return ;
		}
	}
	fseek(fp, 0, SEEK_END);
	fwrite(str, sizeof(BYTE), iLen, fp);
	fclose(fp);
#endif
}

int s_GetFileName(BYTE* pFullFileName,BYTE* abyFileName)
{
//	int iLen;
//	int i;
	BYTE buf[MAX_PATH]={0};

	strcpy((char*)buf,(char*)pFullFileName);
	//char string[] = "A string\tof ,,tokens\nand some  more tokens";
	char seps[]   = "\\/";
	char *token;

	//printf( "%s\n\nTokens:\n", pFullFileName );
	/* Establish string and get the first token: */
	token = strtok( (char*)buf, seps );
	while( token != NULL )
	{
		/* While there are tokens in "string" */
		//printf( " %s\n", token );
		/* Get next token: */
		strcpy((char*)abyFileName,token);
		token = strtok( NULL, seps );
	}

	if (strlen((char*)abyFileName)<1)
	{
		return 2;
	}

	return 0 ;
}


int SmartProtocol::protocol_send_FileFrame(unsigned char* filedata, unsigned int filedatasize, int frameNo, int frameRemain)
{
	unsigned char m_ret;
	unsigned char m_ret_data[1024];
	unsigned char m_senddata[2048];
	unsigned int m_ret_len;
	unsigned int mcount,i;
	unsigned char checksum;
	memset(m_ret_data, 0x00, sizeof(m_ret_data));
	memset(m_senddata, 0x00, sizeof(m_senddata));
	mcount = 0;
	checksum = 0;

	m_senddata[mcount++] = 0x36; //command Id
	m_senddata[mcount++] = 0x00;
	m_senddata[mcount++] = 0x00;   //The total length of the command and need to modify at final!

	m_senddata[mcount++] = frameNo & 0x000000ff;   //Current Frame Number
	m_senddata[mcount++] = (frameNo>>8) & 0x000000ff;

	m_senddata[mcount++] = frameRemain & 0x000000ff;   //Current Frame Number
	m_senddata[mcount++] = (frameRemain>>8) & 0x000000ff;

	for(i = 0; i < filedatasize; i++)
		checksum = checksum^filedata[i];

	m_senddata[mcount++] = checksum;

	memcpy(m_senddata+mcount, filedata, filedatasize);
	mcount = mcount+filedatasize;

	m_senddata[1] = (mcount-3) & 0x000000ff;
	m_senddata[2] = ((mcount-3)>>8) & 0x000000ff;

	m_ret = protocol_cmd(m_senddata, mcount, m_ret_data, &m_ret_len, 1, 1000);
	if(m_ret != SIGLIB_RET_SUCCESS)
		return m_ret;
	if(m_ret_data[0] != 0x36 || m_ret_data[3]!=0x00)
		m_ret = SIGLIB_RET_RESPONDERROR;
	return m_ret;
}

int SmartProtocol::protocol_sendfile(unsigned char* filepath, unsigned char Category)
{
	unsigned long filesize;
	unsigned int frame_number,i;
	unsigned char m_ret;
	unsigned char frame_buffer[SENDFILE_FRAME_MAXDATA+1];
	unsigned char filename[256] = {0};
	unsigned int framesize;
//	TCHAR Tfilename[MAX_PATH];

	FILE* fp;

	g_percent = 0;
	//strcpy(Tfilename,(char*)filename);
	fp = fopen((char*)filepath,"rb");
	if (NULL == fp)
	{	
		//std::cout << "Can open " << filepath << std::endl;
		return -1;
	}

	fseek(fp,0,SEEK_END);
	filesize = ftell(fp);
	if (filesize<=0)
	{
		fclose(fp);
		//std::cout << filepath << "size is 0"<< std::endl;
		return 0;
	}

	fseek(fp, 0, SEEK_SET);

	if(filesize%SENDFILE_FRAME_MAXDATA > 0)
		frame_number = filesize / SENDFILE_FRAME_MAXDATA + 1;
	else
		frame_number = filesize / SENDFILE_FRAME_MAXDATA;
	//step2: send file head, start process
	//display_title_1("SEND FILE HEAD");
	//pdlg->DoStep(IPK_UP_SENDF_H,4);
	printf("%02d%%", fit_percent(g_percent,frame_number));
	//pdlg->m_progctl_percent.SetPos(fit_percent(g_percent,frame_number));
	s_GetFileName(filepath, filename);
	//std::cout << "file name " << filename << std::endl;
	m_ret = protocl_send_FileHead(filename, Category, filesize);
	if(m_ret != SIGLIB_RET_SUCCESS)
	{
		//pdlg->DoStep(IPK_UP_SENDF_H,5);
		fclose(fp);
		printf("\b\b\b\b");
		return m_ret;
	}
	else
	{
		//pdlg->DoStep(IPK_UP_SENDF_H,6);
	}
	//step3: split file into frame and send one by one
	for(i = 0; i<frame_number; i++)
	{

		if(i == frame_number-1)
			framesize = filesize - (SENDFILE_FRAME_MAXDATA*i);
			//framesize = filesize% SENDFILE_FRAME_MAXDATA;
		else
			framesize = SENDFILE_FRAME_MAXDATA;
		fread(frame_buffer, framesize, 1, fp);
		
		//do
		{
			thisSaveSend(frame_buffer,framesize);
			printf("\b\b\b\b%02d%%", fit_percent(i+1,frame_number));
			m_ret = protocol_send_FileFrame(frame_buffer, framesize, i+1, frame_number-i-1);
			//Sleep(100);
		}
		while(m_ret == SIGLIB_RET_RESPONDERROR);

		if(m_ret != SIGLIB_RET_SUCCESS)
		{
			fclose(fp);
			//display_title_1("Send Error Exit");
			//Sleep(100);
			printf("\b\b\b\b");
			return m_ret;
		}
	}
	//SaveSend((BYTE*)"\r\nEnd\r\n",7);
	fclose(fp);
	printf("\b\b\b\b");
	return SIGLIB_RET_SUCCESS;
}



int SmartProtocol::protocol_update_files(unsigned char* filename,unsigned char ucCategory) 
{
	unsigned char m_ret;
	unsigned char m_ret_data[1024];
	unsigned char m_senddata[64];
	unsigned int m_ret_len;
	unsigned int mcount,len;
	//unsigned char Status = m_ret_data[3];

	memset(m_ret_data, 0x00, sizeof(m_ret_data));
	memset(m_senddata, 0x00, sizeof(m_senddata));
	mcount = 0;
	m_senddata[mcount++] = 0x3A; //command Id
	m_senddata[mcount++] = 0x00;
	m_senddata[mcount++] = 0x00;   //The total length of the command and need to modify at final!
	m_senddata[mcount++] = ucCategory;   //Category, reserve for future use


	len = (unsigned int)strlen((char*)filename);
	
	if(len>16) len=16;
	memcpy(m_senddata + mcount, filename, len); //filename, 16 bytes
	//TRACE(TEXT("%d-%s\r\n"),len,filename);

	mcount = mcount+16;

	//put full data length
	m_senddata[1] = (mcount-3) & 0x000000ff;
	m_senddata[2] = ((mcount-3)>>8) & 0x000000ff;
	//ttestall(MORPHO, );
	//ttestall(MORPHO, 200);
	m_ret = protocol_cmd(m_senddata, mcount, m_ret_data, &m_ret_len, 1, 1000*20);
	//ttestall(MORPHO, 7);
	//printf("m_ret=%d\n", m_ret);
	if(m_ret != SIGLIB_RET_SUCCESS)
	{
		//TRACE(TEXT("m_ret=%d"),m_ret);
		//ttestall(MORPHO, 6);
		return m_ret;
	}
	if(m_ret_data[0] != 0x3A)
	{
		//TRACE(TEXT("m_ret_data=%d"),m_ret_data[0]);
		m_ret = SIGLIB_RET_RESPONDERROR;
	}
	//printf("xx\n");
	if(MAKEWORD(m_ret_data[0],m_ret_data[1]) > 0 && m_ret_data[3] != 0x00 ) {
		return m_ret_data[3];
	}
	//printf("m_ret_data[4]=%s\n", &m_ret_data[4]);
	if (ucCategory == SIGLIB_FILE_CATAGORY_BUS_CONFIG && memcmp(&m_ret_data[4], "new", 3) != 0) {
		return CONSOLE_ERROR_FIRMWARE_DO_NOT_SUPPOR;
	}
	//ttestall(MORPHO, 6);
	return m_ret;
}

int SmartProtocol:: update_ipk(char *ipk_path) {
	int ret;
	unsigned char filename[256] = {0};
	s_GetFileName((unsigned char *)ipk_path, filename);
	ret = protocol_sendfile((unsigned char *)ipk_path, SIGLIB_FILE_CATAGORY_IPK);
	if (ret != SIGLIB_RET_SUCCESS) {
		return ret;
	}
	ret = protocol_update_files(filename, SIGLIB_FILE_CATAGORY_IPK);
	if (ret != SIGLIB_RET_SUCCESS) {
		return ret;
	}

	return SIGLIB_RET_SUCCESS;
}


int SmartProtocol:: update_config(char *config_path) {
	int ret;
	unsigned char filename[256] = {0};
	s_GetFileName((unsigned char *)config_path, filename);
	ret = protocol_sendfile((unsigned char *)config_path, SIGLIB_FILE_CATAGORY_BUS_CONFIG);
	if (ret != SIGLIB_RET_SUCCESS) {
		return ret;
	}
	ret = protocol_update_files(filename, SIGLIB_FILE_CATAGORY_BUS_CONFIG);
	if (ret != SIGLIB_RET_SUCCESS) {
		return ret;
	}

	return SIGLIB_RET_SUCCESS;
}


int SmartProtocol::print(char* buff, int len) {

	unsigned char m_ret;
	unsigned char m_ret_data[1024];
	unsigned int m_ret_len = 0;

	memset(m_ret_data, 0x00, sizeof(m_ret_data));	
	m_ret = protocol_cmd((unsigned char*)buff, len, m_ret_data, &m_ret_len, 1, 500);
	if(m_ret != SIGLIB_RET_SUCCESS)
	{
		return m_ret;
	}
	if(m_ret_data[0] != 0x92)
		m_ret = SIGLIB_RET_RESPONDERROR;
	return m_ret;
}