#include <stdio.h>
#include <stdlib.h>

#include "usb.h"
#include "padUSB.h"

/* the device's vendor and product id */
#define MY_VID 0x079B
#define MY_PID 0x05A0

/* the device's endpoints */
#define EP_IN 0x81
#define EP_OUT 0x02

#define BUF_SIZE 64


usb_dev_handle *g_usb_dev = NULL;

static usb_dev_handle *open_dev(void)
{
  struct usb_bus *bus;
  struct usb_device *dev;

  for(bus = usb_get_busses(); bus; bus = bus->next) 
    {
      for(dev = bus->devices; dev; dev = dev->next) 
        {
          if(dev->descriptor.idVendor == MY_VID
             && dev->descriptor.idProduct == MY_PID)
            {
              return usb_open(dev);
            }
        }
    }
  return NULL;
}


int if_usb(int flag) {
	
	if(flag ==0) {
		return 0;
	}
	usb_init(); /* initialize the library */
	usb_find_busses(); /* find all busses */
	usb_find_devices(); /* find all connected devices */

	if(!(g_usb_dev = open_dev()))
	{
		//printf("error: device not found!\n");
		return 0;
	}

	if(usb_claim_interface(g_usb_dev, 0) < 0)
	{
		//printf("error: claiming interface 0 failed\n");
		usb_close(g_usb_dev);
		return 0;
	}
	return 1;
}

static int smartad_comm_reset(void) {
    int ret;

    if (g_usb_dev == NULL) {
        return 0;
    }

    ret = usb_reset(g_usb_dev);
    if (ret < 0) {
        return ret;
    }

    Sleep(3*1000);

    return 0;
}

int lib_usb_reset(void){
	return smartad_comm_reset();
}

int lib_usb_open(void){
	if(g_usb_dev != NULL){
		return 1;
	}

	usb_init(); /* initialize the library */
	usb_find_busses(); /* find all busses */
	usb_find_devices(); /* find all connected devices */

	if(!(g_usb_dev = open_dev()))
	{
		printf("error: device not found!\n");
		return -1;
	}

	if(usb_claim_interface(g_usb_dev, 0) < 0)
	{
		printf("error: claiming interface 0 failed\n");
		usb_close(g_usb_dev);
		return -1;
	}
	return 1;
}

int lib_usb_write(char *send_buf, int send_size, int send_timeout)
{
	int i = 0, ret = 0;
	ret = usb_bulk_write(g_usb_dev, EP_OUT, send_buf, send_size,send_timeout);
	return ret;
}

int lib_usb_read(char *rec_buf, int rec_size, int rec_timeout)
{
	int i = 0, ret = 0;
	ret = usb_bulk_read(g_usb_dev, EP_IN, rec_buf, rec_size, rec_timeout);
	return ret;
}



int lib_usb_transfer(char *send_buf, int send_size, int send_timeout, char *rec_buf, int *rec_size, int rec_timeout){
	int send_fix_timeout = 10000, rec_fix_timeout = 60*0000;
	int rec_fix_len = 1024*2;
	int ret = 0;
//	char *send_local_buf = (char *)calloc(1, 1024*2);
	char *rec_local_buf = (char *)calloc(1, 1024*2);

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
	
	again:
	ret = usb_bulk_write(g_usb_dev, EP_OUT, send_buf, send_size,send_fix_timeout);
	if (ret < 0) {
        	ret = smartad_comm_reset();
        	if (ret < 0) {
					if(rec_local_buf != NULL){
						free(rec_local_buf);
						rec_local_buf = NULL;
					}
            		return ret;
       	 }
       	 goto again;
	}

	ret = usb_bulk_read(g_usb_dev, EP_IN, rec_local_buf, rec_fix_len,rec_fix_timeout);
	if (ret < 0) {
        	ret = smartad_comm_reset();
        	if (ret < 0) {
					if(rec_local_buf != NULL){
						free(rec_local_buf);
						rec_local_buf = NULL;
					}
            		return ret;
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
	
	return 1;
}

/*
int lib_usb_send_data(char *send_buf, int send_size, int time_out){
	int send_timeout = 1000;
	int ret = 0;

	if(send_buf == NULL){
		return -1;
	}

	if(send_size <= 0){
		return -2;
	}


	if(time_out >= 1000){
		send_timeout = time_out;
	}
	
	if(g_usb_dev == NULL){
		ret = lib_usb_open();
		if(ret < 0){
			return -3;
		}
	}
	
again:
	ret = usb_bulk_write(g_usb_dev, EP_OUT, send_buf, send_size,send_timeout);
	if (ret < 0) {
        	ret = smartad_comm_reset();
        	if (ret < 0) {
            		return ret;
       	 }
       	 goto again;
	}
	return 1;
}

int lib_usb_rec_data(char *rec_buf, int rec_size, int time_out){
	int rec_timeout = 5000;
	int ret = 0;

	if(rec_buf == NULL){
		return -1;
	}

	if(rec_size <= 0){
		return -2;
	}


	if(time_out >= rec_timeout){
		rec_timeout = time_out;
	}
	
	if(g_usb_dev == NULL){
		ret = lib_usb_open();
		if(ret < 0){
			return -3;
		}
	}
	
again:
	ret = usb_bulk_read(g_usb_dev, EP_IN, rec_buf, rec_size,rec_timeout);
	if (ret < 0) {
        	ret = smartad_comm_reset();
        	if (ret < 0) {
            		return ret;
        	}
        goto again;
    }
	return 1;
}
*/



#if 0
int lib_usb_transfer_data(char *send_buf, int send_size,char *reci_buf, int reci_size){
	if(send_buf == NULL || reci_buf == NULL){
		return -1;
	}

	int send_timeout = 1000;
	int reci_timeout = 5000;
	int ret = 0;
	
	if(g_usb_dev == NULL){
		ret = lib_usb_open();
		if(ret < 0){
			return -3;
		}
	}
	
	if(send_size <= 0){
		ret = usb_bulk_write(g_usb_dev, EP_OUT, send_buf, send_size,send_timeout);
		if(ret < 0){
			return -4;
		}
	}
	Sleep(10);
	if(reci_size <=0){
		ret = usb_bulk_read(g_usb_dev, EP_IN, reci_buf, reci_size,reci_timeout);
		if(ret < 0){
			return -5;
		}
	}
	return 1;
}
#endif

void lib_usb_close(void){
	if(g_usb_dev  != NULL){
 	 usb_release_interface(g_usb_dev, 0);
	 usb_close(g_usb_dev);
	 g_usb_dev = NULL;
		}
	 return;
}