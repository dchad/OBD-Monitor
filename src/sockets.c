/* 
   sockets.c
   
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: UDP server and client functions. 


   Date: 18/12/2017
   
*/

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

int c_sock, s_sock;
unsigned int length;
struct sockaddr_in obd_server, obd_client, from;
struct hostent *hp;
int ecu_connected;
int ecu_auto_connect;


#ifdef _WINSOCK

WSADATA wsaData;
int iResult;
unsigned long iMode = 1;

int init_client_socket(char *server, char *port)
{
   iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
   if (iResult != NO_ERROR)
      printf("init_client_socket() <ERROR>: WSAStartup() failed with error: %ld\n", iResult);

   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) 
      printf("init_client_socket() <ERROR>: socket creation failed.\n");

   iResult = ioctlsocket(sock, FIONBIO, &iMode);
   if (iResult != NO_ERROR)
      printf("init_client_socket() <ERROR>: ioctlsocket failed with error: %ld\n", iResult);

   obd_server.sin_family = AF_INET;
   hp = gethostbyname(server);
   if (hp == 0) 
      printf("init_client_socket() <ERROR>: Unknown host -> %s.\n", server);

   bcopy((char *)hp->h_addr, (char *)&obd_server.sin_addr, hp->h_length);
   obd_server.sin_port = htons(atoi(port));
   length = sizeof(struct sockaddr_in);

   return(sock);
}

int init_server_socket(char *port)
{
   /* TODO:  */

   return(sock);
}


#else

/* NOT WINSOCK */

int init_client_socket(char *server, char *port)
{
   c_sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (c_sock < 0) 
      printf("init_client_socket() <ERROR>: socket creation failed.\n");

   obd_server.sin_family = AF_INET;
   hp = gethostbyname(server);
   if (hp == 0) 
      printf("init_client_socket() <ERROR>: Unknown host -> %s.\n", server);

   bcopy((char *)hp->h_addr, (char *)&obd_server.sin_addr, hp->h_length);
   obd_server.sin_port = htons(atoi(port));
   length = sizeof(struct sockaddr_in);

   return(c_sock);
}

int init_server_socket(char *port)
{
   struct sockaddr_in server;
   
   s_sock = socket(AF_INET, SOCK_DGRAM, 0);

   if (s_sock < 0) 
      printf("init_server_socket() <ERROR>: Opening socket");
   
   length = sizeof(struct sockaddr_in);
   memset(&server, 0, length);

   server.sin_family=AF_INET;
   server.sin_addr.s_addr=INADDR_ANY;
   server.sin_port=htons(atoi(port));
   
   if (bind(s_sock, (struct sockaddr *)&server, length) < 0) 
      printf("init_server_socket() <ERROR>: binding");
      
   return(s_sock);
}



#endif


int send_ecu_msg(char *query)
{
   int n;

   n = sendto(c_sock,query,strlen(query),0,(const struct sockaddr *)&obd_server,length);
   if (n < 0) 
   {
      printf("send_ecu_msg() <ERROR>: Sendto failed.\n");
   }
   else
   {  
      /* TODO: Write message to log file. */
      /* printf("send_ecu_msg() - SENT ECU Message: %s", buffer); */
   }
   
   return n;
}

int recv_ecu_msg(char *msg)
{
   int n;

   memset(msg,0,256);

   n = recvfrom(c_sock,msg,256,MSG_DONTWAIT,(struct sockaddr *)&from,&length);
   /* We are not blocking on recv now. */
   
   /*
   if (n > 0)
   {
      printf("recv_ecu_msg() - RECV ECU Message: %s", buffer); 
      strncpy(msg, buffer, n); 
   }
   */
   
   return(n);
}


int init_obd_comms(char *obd_msg)
{
   int n;

   n = sendto(c_sock,obd_msg,strlen(obd_msg),0,(const struct sockaddr *)&obd_server,length);

   if (n <= 0) 
   {
      printf("init_obd_comms() - <ERROR>: Sendto failed.\n");
   }
   else
   {
      printf("init_obd_comms() - SENT OBD Message: %s", obd_msg);
   }
   
   
   return(n);
}


int server_connect()
{
   int result;
   
   /* First set up UDP communication with the server process
      and check connection to the OBD interface. */
   result = init_client_socket("127.0.0.1", "8989");
   
   if (result <= 0)
   {
      printf("ecu_connect() <ERROR>: Failed to connect to OBD server.\n");
      ecu_connected = 0;
   }
   else
   {
      printf("ecu_connect() <INFO>: Connected to OBD server.\n");
      ecu_connected = 1;
   }
   
   /* Do this in the GUI/Client process.
   else
   {
      result = init_obd_comms("ATI\n", rcv_msg_buf);
      if (result <= 0)
      {
         printf("ecu_connect() <ERROR>: Failed to connect to OBD interface.\n");
      }
      else
      {
         ecu_connected = 1;
         if (strlen(protocol_req) > 5)
         {
            send_ecu_msg(protocol_req);
         }
      }
   }
   */
   
   return(result);
}

int get_ecu_connected()
{
   return(ecu_connected);
}

void set_ecu_connected(int cstatus)
{
   ecu_connected = cstatus;
   return;
}
