#include "port.h"
#include "Cserial.h"
#include "padUSB.h"
#include <string.h>
#include <stdio.h>
#include <iostream>

SmartPort::SmartPort(SmartPortType t, int port_NUM, char *param) {
	portType = t;
	Open(port_NUM, param);
}

SmartPort::~SmartPort(void)
{
	Close();
}

int SmartPort::Reset(void) {
	if (portType == RS232) {
		serial.SerReset();
	} else if (portType == USB) {
		//lib_usb_reset();
	}

	return 0;
}

int SmartPort::Close() {
	if (portType == RS232) {
		serial.SerReset();
	} else if (portType == USB) {
		lib_usb_close();
	}

	return 0;
}

int SmartPort::Open(int port_NUM, char *param) {
	if (portType == RS232) {
		serial.SerOpen(port_NUM, param);
	} else if (portType == USB) {
		lib_usb_open();
	}


	return 0;
}


int SmartPort::Read(unsigned char *pszBuf , int RecvCnt , int TimeOut) {
	int  rec_fix_timeout = 60*0000;
	if (portType == RS232) {
		serial.SerRead(pszBuf, RecvCnt, TimeOut);
	} else if (portType == USB) {
		lib_usb_read((char *)pszBuf, RecvCnt, rec_fix_timeout);
	}


	return 0;
}

int SmartPort::Write(unsigned char *pszBuf , int SendCnt , int TimeOut) {
	int send_fix_timeout = 10000;
	if (portType == RS232) {
		serial.SerWrite(pszBuf, SendCnt, TimeOut);
	} else if (portType == USB) {
		lib_usb_write((char *)pszBuf, SendCnt, send_fix_timeout);
	}
	return 0;
}


int SmartPort::Transfer(unsigned char *send_buf, int send_size, int send_timeout, unsigned char *rec_buf, int *rec_size, int rec_timeout) {

	int send_fix_timeout = 10000, rec_fix_timeout = 30*10000;
	int rec_fix_len = 1024*2;
	int ret = 0;
	int serial_len = 0, frame_len = 0;
	unsigned char serial_buff[1024+128] = {0};
//	char *send_local_buf = (char *)calloc(1, 1024*2);
	char *rec_local_buf = (char *)calloc(1, 1024*2);

	if (portType == USB) 
	{
		if(send_buf == NULL || rec_buf == NULL || rec_size == NULL){
			if(rec_local_buf != NULL){
				free(rec_local_buf);
				rec_local_buf = NULL;
			}
			return -1;
		}

		if(send_size <= 0 || *rec_size <= 0 || *rec_size >= 1024*2){
			if(rec_local_buf != NULL){
				free(rec_local_buf);
				rec_local_buf = NULL;
			}
			return -2;
		}

		if(send_timeout >= 1000){
			send_fix_timeout = send_timeout;
		}

		if(rec_timeout >= rec_fix_timeout){
			rec_fix_timeout = rec_timeout;
		}
	/*
		if(g_usb_dev == NULL){
			ret = lib_usb_open();
			if(ret < 0){
				if(rec_local_buf != NULL){
					free(rec_local_buf);
					rec_local_buf = NULL;
				}
				return -3;
			}
		}
	*/	
		again:
		ret = lib_usb_write((char *)send_buf, send_size,send_fix_timeout);
		if (ret < 0) {
        		ret = lib_usb_reset();
        		if (ret < 0) {
						if(rec_local_buf != NULL){
							free(rec_local_buf);
							rec_local_buf = NULL;
						}
            			return -1;
       			}
       			goto again;
		}

		ret = lib_usb_read(rec_local_buf, rec_fix_len,rec_fix_timeout);
		if (ret < 0) {
        		ret = lib_usb_reset();
        		if (ret < 0) {
						if(rec_local_buf != NULL){
							free(rec_local_buf);
							rec_local_buf = NULL;
						}
            			return -1;
        		}
			goto again;
		}else{
			if(ret > *rec_size){
				memcpy(rec_buf, rec_local_buf, ret);
			}else{
				memcpy(rec_buf, rec_local_buf, *rec_size);
			}
			*rec_size = ret;
		}
	
		if(rec_local_buf != NULL){
			free(rec_local_buf);
			rec_local_buf = NULL;
		}

	} else if (portType == RS232) {
		serial.SerReset();
		ret = serial.SerWrite(send_buf, send_size, send_timeout);
		if (ret < 0) {
				return -1;
		}

		memset(serial_buff, 0 ,sizeof(serial_buff));
		ret = serial.SerRead(serial_buff, 4, rec_timeout);
		if (ret)
		{
		//TRACE("error1[%d]\r\n",iret);
			return -1;
		}
		
		memcpy(&rec_buf[serial_len], serial_buff, 4);
		serial_len = 4;

		memset(serial_buff, 0 ,sizeof(serial_buff));
		ret = serial.SerRead(serial_buff, 8 , 100);
		if (ret < 0) {
			return -1;
		}
		
		memcpy(&rec_buf[serial_len], serial_buff, 8);
		serial_len += 8;
		frame_len = MAKELONG( MAKEWORD(serial_buff[0],serial_buff[1]),MAKEWORD(serial_buff[2],serial_buff[3]));

		memset(serial_buff, 0 ,sizeof(serial_buff));
		ret = serial.SerRead(serial_buff, frame_len + 2 , 200);
		if (ret < 0) {
			return -1;
		}

		memcpy(&rec_buf[serial_len], serial_buff, frame_len + 2);
		

		serial.SerReset();
	}
	return 0;
}