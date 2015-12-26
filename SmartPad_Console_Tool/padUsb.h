#ifndef _LIB_PAD_H_
#define _LIB_PAD_H_

int lib_usb_open(void);
//int lib_usb_send_data(char *send_buf, int send_size, int time_out);
//int lib_usb_rec_data(char *rec_buf, int rec_size, int time_out);
int lib_usb_transfer(char *send_buf, int send_size, int send_timeout, char *rec_buf, int* rec_size, int rec_timeout);
void lib_usb_close(void);
int lib_usb_reset(void);
int lib_usb_write(char *send_buf, int send_size, int send_timeout);
int lib_usb_read(char *rec_buf, int rec_size, int rec_timeout);
int if_usb(int flag);

#endif