/* 
   Project: OBD-II Monitor (On-Board Diagnostics)
   
   
   File: test_server.c
   
   
   Author: Derek Chadwick

   Description: A test set for the UDP server that communicates with vehicle
                engine control units via an OBD-II interface to obtain 
                engine status and fault codes. Can also be used as an ECU
                communications logging function when the GUI is not required.
                

   Date: 03/01/2018
   
*/

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

const char *OBD_Protocol_List[] = {
"OBD 0 - Automatic OBD-II Protocol Search",
"OBD 1 - SAE J1850 PWM (41.6 kbaud)(Ford)",
"OBD 2 - SAE J1850 VPW (10.4 kbaud)(GM, Isuzu)",
"OBD 3 - IS0 9141-2 (5 baud init, 10.4 kbaud)(Chrysler)",
"OBD 4 - ISO 14230-4 KWP2000 (5-baud init.)",
"OBD 5 - IS0 14230-4 KWP2000 (Fast init.)",
"OBD 6 - IS0 15765-4 CAN (11 bit ID, 500 kbaud)",
"OBD 7 - IS0 15765-4 CAN (29 bit ID, 500 kbaud)",
"OBD 8 - IS0 15765-4 CAN (11 bit ID, 250 kbaud)",
"OBD 9 - IS0 15765-4 CAN (29 bit ID, 250 kbaud)",
"OBD A - SAE J1939 CAN (29 bit ID, 250 kbaud)",
"OBD B - USER1 CAN (11 bit ID, 125 kbaud)",
"OBD C - USER2 CAN (11 bit ID, 50 kbaud)"
};

int main(int argc, char *argv[])
{
   struct timespec reqtime;
   char protocol_req[256];
   char recv_msg[256];
   int ii;
   
   
   memset(recv_msg, 0, 256);
   memset(protocol_req, 0, 256);
   reqtime.tv_sec = 1;
   reqtime.tv_nsec = 0;   


   if (argc < 2) /* Get protocol number from command line. */
   {
      strcpy(protocol_req, "ATTP 0\n");
   }
   else
   {
      strncpy(protocol_req, "ATTP %c\n", argv[1][0]);
   }
   
   if (server_connect(recv_msg, protocol_req) > 0) /* Sockets Module Connect Function. */
   {
   
      send_ecu_msg("ATRV\n"); 
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("ATRV: %s", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("ATDP\n");  /* Get OBD protocol name from interface. */
      nanosleep(&reqtime, NULL); 
      recv_ecu_msg(recv_msg);
      printf("ATDP: %s", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("ATRV\n");  /* Get battery voltage from interface. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("ATRV: %s", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("09 02\n"); /* Get vehicle VIN number. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("VIN: %s", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("09 0A\n"); /* Get ECU name. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("ECUName: %s", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("01 01\n"); /* Get DTC Count and MIL status. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("MIL: %s", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("01 00\n"); /* Get supported PIDs 1 - 32 for MODE 1. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("PID01: %s", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("09 00\n"); /* Get supported PIDs 1 - 32 for MODE 9. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("PID09: %s", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("03\n");      /* Get DTCs that are set. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("DTC: %s", recv_msg);
      memset(recv_msg, 0, 256);
   }
   else
   {
      printf("Connection to ECU failed.\n");
      exit(-1);
   }

   reqtime.tv_sec = 1;
   reqtime.tv_nsec = 0; 
      

   send_ecu_msg("01 0C\n"); /* Engine RPM */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_msg(recv_msg);
   printf("RPM: %s", recv_msg);
   memset(recv_msg, 0, 256);

   send_ecu_msg("01 0D\n"); /* Vehicle Speed */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_msg(recv_msg);
   printf("VS: %s", recv_msg);
   memset(recv_msg, 0, 256);

   send_ecu_msg("01 0A\n"); /* Fuel Pressure */
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_msg(recv_msg);
   printf("FP: %s", recv_msg);
   memset(recv_msg, 0, 256);

   send_ecu_msg("01 0B\n"); /* MAP Pressure */
   nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
   recv_ecu_msg(recv_msg);
   printf("MAP: %s", recv_msg);
   memset(recv_msg, 0, 256);

   send_ecu_msg("01 5E\n"); /* Fuel Flow Rate */  
   nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
   recv_ecu_msg(recv_msg);
   printf("FFR: %s", recv_msg);
   memset(recv_msg, 0, 256);

   send_ecu_msg("01 05\n"); /* Coolant Temperature */
   nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
   recv_ecu_msg(recv_msg);
   printf("ECT: %s", recv_msg);
   memset(recv_msg, 0, 256);

   send_ecu_msg("01 2F\n"); /* Fuel Tank Level */
   nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
   recv_ecu_msg(recv_msg);
   printf("FTL: %s", recv_msg);
   memset(recv_msg, 0, 256);

   send_ecu_msg("01 0F\n"); /* Intake Air Temperature */
   nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
   recv_ecu_msg(recv_msg);
   printf("IAT: %s", recv_msg);
   memset(recv_msg, 0, 256);

   send_ecu_msg("01 5C\n"); /* Oil Temperature */    
   nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
   recv_ecu_msg(recv_msg);
   printf("OILT: %s", recv_msg);
   memset(recv_msg, 0, 256);
      
   for (ii = 0; ii < 10; ii++)
   {
      /* TODO: send a bunch of messages. 
      send_ecu_msg("01 0C\n"); */
      send_ecu_msg("03\n");
      nanosleep(&reqtime, NULL);
      while (recv_ecu_msg(recv_msg) > 0)
      {
         printf("ECU: <%d> %s", ii, recv_msg);
         memset(recv_msg, 0, 256);
      }
   }
   
   
   printf("Server Test Exiting...\n");
   
   return(0);
}

