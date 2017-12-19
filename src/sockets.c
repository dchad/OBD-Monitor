/* 
   sockets.c
   
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: UDP server and client functions. 


   Date: 18/12/2017
   
*/

#include "obd_monitor.h"
#include "protocols.h"

int sock;
unsigned int length;
struct sockaddr_in obd_server, from;
struct hostent *hp;
int ecu_connected;
int ecu_auto_connect;

int init_server_comms(char *server, char *port)
{
   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) 
      printf("init_server_comms() <ERROR>: socket creation failed.\n");

   obd_server.sin_family = AF_INET;
   hp = gethostbyname(server);
   if (hp == 0) 
      printf("init_server_comms() <ERROR>: Unknown host -> %s.\n", server);

   bcopy((char *)hp->h_addr, (char *)&obd_server.sin_addr, hp->h_length);
   obd_server.sin_port = htons(atoi(port));
   length = sizeof(struct sockaddr_in);

   return(sock);
}

int send_ecu_msg(char *query)
{
   char buffer[256];
   int n;

   memset(buffer,0,256);
   sprintf(buffer,"%s",query);

   n = sendto(sock,buffer,strlen(buffer),0,(const struct sockaddr *)&obd_server,length);
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

int recv_ecu_msg()
{
   char buffer[256];
   int n;

   memset(buffer,0,256);

   n = recvfrom(sock,buffer,256,MSG_DONTWAIT,(struct sockaddr *)&from,&length);
   /* We are not blocking on recv now. */
   if (n > 0)
   {
      /* printf("recv_ecu_msg() - RECV ECU Message: %s", buffer); */
      if (parse_obd_msg(buffer) > 0)
      {
         /* TODO: Write message to text view widget and log file. */
         update_comms_log_view(buffer);
      }
   }

   return(n);
}


int init_obd_comms(char *obd_msg)
{
   int n;
   char buffer[256];

   n = sendto(sock,obd_msg,strlen(obd_msg),0,(const struct sockaddr *)&obd_server,length);

   if (n < 0) 
   {
      printf("init_obd_comms() - <ERROR>: Sendto failed.\n");
   }
   else
   {
      printf("init_obd_comms() - SENT OBD Message: %s", obd_msg);
   }
   
   usleep(OBD_WAIT_TIMEOUT);

   memset(buffer,0,256);

   n = recvfrom(sock,buffer,256,MSG_DONTWAIT,(struct sockaddr *)&from,&length);
   
   if (n > 0)
   {
      printf("init_obd_comms() - RECV OBD Message: %s", buffer);  
      update_comms_log_view(buffer);
      parse_obd_msg(buffer); /* TODO: write log message. */
   }
   
   return(n);
}


int ecu_connect()
{
   int result;
   
   /* First set up UDP communication with the server process
      and check connection to the OBD interface. */
   result = init_server_comms("127.0.0.1", "8989");
   if (result < 0)
   {
      printf("auto_connect() <ERROR>: Failed to connect to OBD server.\n");
   }
   else
   {
      result = init_obd_comms("ATI\n");
      if (result <= 0)
      {
         printf("auto_connect() <ERROR>: Failed to connect to OBD interface.\n");
      }
      else
      {
         ecu_connected = 1;
         send_ecu_msg("ATRV\n");
      }
   }
   
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
