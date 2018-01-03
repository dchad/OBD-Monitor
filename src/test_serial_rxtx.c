
/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   File: test_serial_rxtx.c

   Author: Derek Chadwick

   Description: Serial communications loopback test function for USB to RS232
                converter modules such as the FTDI232. 
                
   Usage:
                1. Connect a wire between the converter module transmit and
                   receive pins, then insert the module into a USB port. 
                   
                2. Check the kernel module is loaded, for example:
                
                   dmesg | grep "FTDI"
                   
                3. Check for user read/write permission on the device.
                   
                   ls -la /dev/ttyUSB0
                   
                   or
                   
                   chmod +s serial_test
                   
                4. Run the serial loopback test:
                
                   ./test_serial

                

   Date: 30/11/2017
   
*/




#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "rs232.h"

char tx_msgs[2][512] = {
   "Serial Port Message One!\n",
   "Serial Port Message Two!\n"
};

int main()
{
  int ii, msg_num=0, n_bytes=0,
  cport_nr=0,        /* /dev/ttyS0 (COM1 on windows) */
  bdrate=9600;       /* 9600 baud */
                     /* Make baud rate configurable. */

  char mode[]={'8','N','1',0};
  char str[2][512];
  unsigned char buf[4096];
  
  cport_nr = RS232_GetPortnr("ttyUSB0");
  if (cport_nr == -1)
  {
     printf("ERROR: Can not get com port number.\n");
     exit(-1);
  }


  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("ERROR: Can not open comport!\n");

    return(0);
  }

  printf("Serial port number: %i\n",cport_nr);

  for (ii = 0; ii < 10; ii++)
  {
    RS232_cputs(cport_nr, str[msg_num]);

    printf("TXD %i bytes: %s", (int)strlen(str[msg_num]), str[msg_num]);

    usleep(100000);  /* sleep for 1 Second */

    msg_num++;

    msg_num %= 2;

    while((n_bytes = RS232_PollComport(cport_nr, buf, 4095)) > 0)
    {
          int idx;

          buf[n_bytes] = 0;   /* always put a "null" at the end of a string! */

          for(idx = 0; idx < n_bytes; idx++)
          {
             if(buf[idx] < 32)  /* replace unreadable control-codes by dots */
             {
                buf[idx] = '.';
             }
          }

          printf("RXD %i bytes: %s\n\n", n_bytes, (char *)buf);

          usleep(100000);  /* sleep for 100 milliSeconds */
    }
  }

  return(0);
}

