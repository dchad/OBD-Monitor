/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A UDP server that communicates with vehicle
                engine control units via an OBD-II interface to obtain 
                engine status and fault codes. 

                Implements two functions:

                1. A UDP datagram server that receives requests for vehicle
                status information from a client application (GUI) and 
                returns the requested information to the client. 

                2. Serial communications to request vehicle status
                information and fault codes from the engine control unit using 
                the OBD-II protocol.

   Date: 30/11/2017
   
*/

#include <ctype.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>

#include "obd_monitor.h"

#include "rs232.h"


int interpreter_ready_status = 0;


void fatal_error(const char *error_msg)
{
    perror(error_msg);
    exit(0);
}

int init_serial_comms(char *interface_name)
{
  int cport_nr=0;        /* /dev/ttyS0 (COM1 on windows) */
  int bdrate=9600;       /* 9600 baud */
  char mode[]={'8','N','1',0};
  
  cport_nr = RS232_GetPortnr(interface_name);
  if (cport_nr == -1)
  {
     printf("ERROR: Can not get com port number.\n");
     exit(-1);
  }

  printf("Serial port number: %i\n",cport_nr);

  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("ERROR: Can not open comport!\n");
    exit(-1);
  }
  
  return(cport_nr);
}

int send_ecu_query(int serial_port, char *ecu_query)
{
    int out_msg_len = 0;
/*  struct timespec reqtime;
    reqtime.tv_sec = 0;
    reqtime.tv_nsec = 1000000; */
    
    out_msg_len = strlen(ecu_query);
    if ((out_msg_len < 1) || (out_msg_len > BUFFER_MAX_LEN))
    {
      printf("ERROR: Bad message length!\n");
      return(0);
    }

    RS232_SendBuf(serial_port, (unsigned char *)ecu_query, out_msg_len);

    printf("TXD %i bytes: %s\n", out_msg_len, ecu_query);

    /* nanosleep(100000);   sleep for 1 millisecond */
    RS232_flushTX(serial_port);

    return(out_msg_len);
}

int recv_ecu_reply(int serial_port, unsigned char *ecu_reply)
{
    int in_msg_len;
    
    interpreter_ready_status = 0;
    
    while ((interpreter_ready_status == 0) && ((in_msg_len = RS232_PollComport(serial_port, ecu_reply, MAX_SERIAL_BUF_LEN)) > 0))
    {
          int idx;

          if (ecu_reply[0] != '>')
          {
             
             ecu_reply[in_msg_len] = 0;   /* always put a "null" at the end of a string! */

             for(idx = 0; idx < in_msg_len; idx++)
             {
                if(ecu_reply[idx] < 32)  /* replace unreadable control-codes by dots */
                {
                   ecu_reply[idx] = '.';
                }
             }

             printf("RXD %i bytes: %s\n", in_msg_len, ecu_reply);
          }
          else
          {
             interpreter_ready_status = 1;
          }
          /* nanosleep(&reqtime, NULL); */
    }

    RS232_flushRX(serial_port); 
    
    return(in_msg_len);
}


void interface_check(int serial_port)
{
   unsigned char recv_msg[MAX_SERIAL_BUF_LEN];
   struct timespec reqtime;
   reqtime.tv_sec = 1;
   reqtime.tv_nsec = 0;
    
   send_ecu_query(serial_port, "ATZ\n"); /* Reset the ELM327 OBD interpreter. */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("ATZ: %s", recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "ATRV\n"); /* Get battery voltage from interface. */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("ATRV: %s", recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "ATDP\n");  /* Get OBD protocol name from interface. */
   nanosleep(&reqtime, NULL); 
   recv_ecu_reply(serial_port, recv_msg);
   printf("ATDP: %s", recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "ATI\n");  /* Get interpreter version ID. */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("ATI: %s", recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "09 02\n"); /* Get vehicle VIN number. */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("VIN: %s", recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "09 0A\n"); /* Get ECU name. */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("ECUName: %s", recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "01 01\n"); /* Get DTC Count and MIL status. */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("MIL: %s", recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "01 00\n"); /* Get supported PIDs 1 - 32 for MODE 1. */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("PID01: %s", recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "09 00\n"); /* Get supported PIDs 1 - 32 for MODE 9. */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("PID09: %s", recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "03\n");      /* Get DTCs that are set. */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("DTC: %s", recv_msg);
   memset(recv_msg, 0, 256);

   return;
}


int main(int argc, char *argv[])
{
   int sock, length, n, serial_port;
   socklen_t from_len;
   struct sockaddr_in server;
   struct sockaddr_in from_client;
   char in_buf[BUFFER_MAX_LEN];
   unsigned char ecu_msg[BUFFER_MAX_LEN];
   struct timespec reqtime;
    
   reqtime.tv_sec = 1;
   reqtime.tv_nsec = 0;
    
   if (argc < 2) 
   {
      fprintf(stderr, "ERROR: no port provided.\n");
      exit(0);
   }
   
   open_log_file("./", "obd_server_log.txt");
   
   /* TODO: make serial port configurable, ttyUSB0 is an FTDI232 USB-RS232 Converter Module. */
   serial_port = init_serial_comms("ttyUSB0");
   
   sock = socket(AF_INET, SOCK_DGRAM, 0);

   if (sock < 0) 
      fatal_error("Opening socket");
   
   length = sizeof(server);
   memset(&server, 0, length);

   server.sin_family=AF_INET;
   server.sin_addr.s_addr=INADDR_ANY;
   server.sin_port=htons(atoi(argv[1]));
   
   if (bind(sock, (struct sockaddr *)&server, length) < 0) 
      fatal_error("binding");

   from_len = sizeof(struct sockaddr_in);
   
   while (1) 
   {
       /* Clear the buffers!!! */
       memset(in_buf, 0, BUFFER_MAX_LEN);
       memset(ecu_msg, 0, BUFFER_MAX_LEN);
       
       n = recvfrom(sock, in_buf, BUFFER_MAX_LEN, 0, (struct sockaddr *)&from_client, &from_len);

       if (n < 0) 
          fatal_error("recvfrom");

       /* printf("RXD ECU Query: %s\n", in_buf); */

       /* Now send the query to the ECU interface and get a response. */
       n = send_ecu_query(serial_port, in_buf);
       if (n > 0)
       {
          print_log_entry(in_buf);
       }
       
       nanosleep(&reqtime, NULL);
       
       n = recv_ecu_reply(serial_port, ecu_msg);
       if (n > 0)
       {
          print_log_entry((char *)ecu_msg);
          
          /* Send ECU reply to GUI. */
          n = sendto(sock, ecu_msg, n, 0, (struct sockaddr *)&from_client, from_len);

          if (n  < 0) 
             fatal_error("sendto");
       }
       

   }

   close_log_file();
   
   return(0);
 }

