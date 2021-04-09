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
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef _WINSOCK

#include <winsock.h>

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#endif




#include "obd_monitor.h"

#include "rs232.h"


#define DEFAULT_UDP_PORT 8989


void fatal_error(const char *error_msg)
{
   /* TODO: log error message. */
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
     printf("init_serial_comms() ERROR: Cannot get com port number.\n");
     exit(-1);
  }

  printf("init_serial_comms() Serial port number: %i\n",cport_nr);

  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("innit_serial_comms() ERROR: Cannot open comport!\n");
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
    
    if ((out_msg_len < 1) || (out_msg_len > MAX_BUFFER_LEN))
    {
      printf("send_ecu_query() ERROR: Bad message length!\n");
      return(0);
    }

    RS232_SendBuf(serial_port, (unsigned char *)ecu_query, out_msg_len);

    printf("send_ecu_query() TXD %i bytes: %s\n", out_msg_len, ecu_query);

    /* nanosleep(100000);   sleep for 1 millisecond */
    RS232_flushTX(serial_port);

    return(out_msg_len);
}

int recv_ecu_reply(int serial_port, char *ecu_reply)
{
   int in_msg_len;
   unsigned char in_buf[MAX_SERIAL_BUF_LEN];
   int interpreter_ready_status = 0;
   int msg_idx = 0;

   while (interpreter_ready_status == 0) /* TODO: need a timeout in case lose comms with interpreter. */
   {
      int buf_idx;

      memset(in_buf, 0, MAX_SERIAL_BUF_LEN);

      if ((in_msg_len = RS232_PollComport(serial_port, in_buf, MAX_SERIAL_BUF_LEN)) > 0)
      {
         /* printf("recv_ecu_reply(): RXD00 buf %i bytes: %s\n", in_msg_len, in_buf); */
 
         if (in_buf[0] == '>')
         {
            /* TODO: something wrong here!!! */
            /* ELM327 is ready to receive another request, so exit. */
            /* See ELM327 datasheet for vague details of protocol.  */
            interpreter_ready_status = 1;
            printf("recv_ecu_reply(): RXD01 > Interpreter Ready.\n");
         }
         else
         {
         
            for (buf_idx = 0; buf_idx < in_msg_len; buf_idx++)
            {
               if (in_buf[buf_idx] < 32)   /* Ignore unreadable control-codes except 0x0D message delimiter. */
               {
                  /* if (in_buf[buf_idx] == '\r')  End of message ASCII value 0x0D == \r not ASCII value 0x0A == \n */
                  
                     ecu_reply[msg_idx] = '!'; /* Delimiter between request and response. */
                     msg_idx++;
                  
               }
               else
               {

                  if (in_buf[buf_idx] == '>')
                  {
                     /* ELM327 is ready to receive another request, so exit. */
                     /* See ELM327 datasheet for vague details of protocol.  */
                     interpreter_ready_status = 1;
                     /* DEBUG: view raw messages. */
                     printf("recv_ecu_reply(): RXD02 > Interpreter Ready: %s\n", in_buf);
                  }
                  else
                  {
                     ecu_reply[msg_idx] = in_buf[buf_idx]; /* Add character to the reply message buffer. */
                     msg_idx++;
                  }
               }
            }  
         }
      }
   }

   RS232_flushRX(serial_port); 

   printf("recv_ecu_reply(): RXD buf %i bytes: %s\n", in_msg_len, in_buf);
   printf("recv_ecu_reply(): RXD msg %i bytes: %s\n", msg_idx, ecu_reply);

   return(msg_idx);
}


/* TODO: Temp protocol test function, move to functional test module. */
void interface_check(int serial_port)
{
   char recv_msg[MAX_SERIAL_BUF_LEN];
   /* struct timespec reqtime;
   reqtime.tv_sec = 1;
   reqtime.tv_nsec = 0; */
   
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "ATZ\r\0"); /* Reset the ELM327 OBD interpreter. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("ATZ: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);

   send_ecu_query(serial_port, "ATRV\r\0"); /* Get battery voltage from interface. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("ATRV: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "ATDP\r\0");  /* Get OBD protocol name from interface. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("ATDP: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "ATI\r\0");  /* Get interpreter version ID. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("ATI: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "09 02\r\0"); /* Get vehicle VIN number. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("VIN: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "09 0A\r\0"); /* Get ECU name. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("ECUName: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "01 01\r\0"); /* Get DTC Count and MIL status. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("MIL: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "01 00\r\0"); /* Get supported PIDs 1 - 32 for MODE 1. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("PID01: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "09 00\r\0"); /* Get supported PIDs 1 - 32 for MODE 9. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("PID09: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);
   
   send_ecu_query(serial_port, "03\r\0");      /* Get DTCs that are set. */
   recv_ecu_reply(serial_port, recv_msg);
   printf("DTC: %s\n", recv_msg);
   print_log_entry((char *)recv_msg);
   memset(recv_msg, 0, 256);

   /* nanosleep(&reqtime, NULL);  Sleep for 1 Second. */

   return;
}


int main(int argc, char *argv[])
{
   int sock, length, n, udp_port, serial_port;
   socklen_t from_len;
   struct sockaddr_in server;
   struct sockaddr_in from_client;
   char in_buf[MAX_BUFFER_LEN];
   char log_buf[MAX_BUFFER_LEN+64];
   char ecu_msg[MAX_BUFFER_LEN];
   char *pch;
   
   /*
   struct timespec reqtime;
   reqtime.tv_sec = 1;
   reqtime.tv_nsec = 0;
   */
    
   if (argc < 2) 
   {
      udp_port = DEFAULT_UDP_PORT;
   }
   else
   {
      udp_port = atoi(argv[1]);
   }


   
   open_log_file("./", "obd_server_log.txt");
   
   /* TODO: make serial port configurable, ttyUSB0 is an FTDI232 USB-RS232 Converter Module. */
   serial_port = init_serial_comms("ttyUSB0");
   
   interface_check(serial_port);
   
   
   
   
#ifdef _WINSOCK

   WSADATA wsaData;
   int iResult;
   unsigned long iMode = 1;
   iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
   if (iResult != NO_ERROR)
      printf("init_server_comms() <ERROR>: WSAStartup() failed with error: %ld\n", iResult);

#endif
   
   
   
   sock = socket(AF_INET, SOCK_DGRAM, 0);

   if (sock < 0) 
      fatal_error("Opening socket");
   
   length = sizeof(server);
   memset(&server, 0, length);

   server.sin_family=AF_INET;
   server.sin_addr.s_addr=INADDR_ANY;
   server.sin_port=htons(udp_port);
   
   if (bind(sock, (struct sockaddr *)&server, length) < 0) 
      fatal_error("binding");

   from_len = sizeof(struct sockaddr_in);
   
   while (1) 
   {
       /* Clear the buffers!!! */
       memset(in_buf, 0, MAX_BUFFER_LEN);
       memset(ecu_msg, 0, MAX_BUFFER_LEN);
       
       n = recvfrom(sock, in_buf, MAX_BUFFER_LEN, 0, (struct sockaddr *)&from_client, &from_len);

       if (n < 0) 
          fatal_error("recvfrom");

       /* TODO: do some message vaidation here.
        printf("RXD ECU Query: %s\n", in_buf); */

       /* Now send the query to the interpreter and get a response. */
       n = send_ecu_query(serial_port, in_buf);
       if (n > 0)
       {
          sprintf(log_buf, "main(): TXD - %s", in_buf);
          print_log_entry(log_buf);
       }
       
       /* nanosleep(&reqtime, NULL); */
       
       n = recv_ecu_reply(serial_port, ecu_msg);
       if (n > 3)
       {
          /* Reformat messages before sending to the GUI. 
             ELM327 returns the request message plus the ECU response,
             so break off the request header and only send the ECU response
             to the GUI. 
          */
          if (strstr(ecu_msg, "ERROR") != 0) /* Interpreter sent a data error message, so ignore it. */
          {
             sprintf(log_buf, "main(): DATA ERROR - %s\n", ecu_msg);
             print_log_entry(log_buf);

          }
          else if (ecu_msg[0] == 'A') /* TODO: or 'a' Interpreter AT response message. */
          {
             /* Replace ! with space. */
             replacechar((char *)ecu_msg, '!', ' ');
             sprintf(log_buf, "main(): RXD AT MSG: %s", ecu_msg);
             print_log_entry(log_buf);
             
             /* Send interpreter reply to GUI. */
             n = sendto(sock, ecu_msg, n, 0, (struct sockaddr *)&from_client, from_len);

             if (n  < 0) 
                fatal_error("sendto");
          }
          else if (ecu_msg[0] == '0') /* ELM327 sends the request header plus the ECU response message. */
          {
             /* replacechar((char *)ecu_msg, '!', ' '); */          
             sprintf(log_buf, "main(): RXD ECU MSG: %s", ecu_msg);
             print_log_entry(log_buf);
             
             pch = strtok((char *)ecu_msg,"!"); /* Cut off the header and delimiters. */
             pch = strtok(NULL,"!");
             if (pch != NULL)
             {
                
                /* Send ECU reply to GUI. */
                n = sendto(sock, pch, strlen(pch), 0, (struct sockaddr *)&from_client, from_len);

                if (n  < 0) 
                   fatal_error("sendto");

                printf("main(): Sent ECU msg to GUI: %s\n", pch);
             }
             
          }
          else
          {
             /* Log an error message. */
             sprintf(log_buf, "main(): RXD Unknown ECU Message: %s", ecu_msg);
             print_log_entry(log_buf);
          }
       }
       

   }

   close_log_file();
   
   return(0);
 }

