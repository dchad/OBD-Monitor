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
      strcpy(protocol_req, "ATTP 0\r");
   }
   else
   {
      strncpy(protocol_req, "ATTP %c\r", argv[1][0]);
   }
   
   if (server_connect() > 0) /* Sockets Module Connect Function. */
   {
   
      send_ecu_msg("ATRV\r"); 
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("ATRV: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("ATDP\r");  /* Get OBD protocol name from interface. */
      nanosleep(&reqtime, NULL); 
      recv_ecu_msg(recv_msg);
      printf("ATDP: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("ATRV\r");  /* Get battery voltage from interface. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("ATRV: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("09 02\r"); /* Get vehicle VIN number. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("VIN: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("09 0A\r"); /* Get ECU name. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("ECUName: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("01 01\r"); /* Get DTC Count and MIL status. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("MIL: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("01 00\r"); /* Get supported PIDs 1 - 32 for MODE 1. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("PID01: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("09 00\r"); /* Get supported PIDs 1 - 32 for MODE 9. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("PID09: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      send_ecu_msg("03\r");      /* Get DTCs that are set. */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("DTC: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      
      send_ecu_msg("01 0C\r"); /* Engine RPM */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("RPM: %s\n", recv_msg);
      memset(recv_msg, 0, 256);

      send_ecu_msg("01 0D\r"); /* Vehicle Speed */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("VS: %s\n", recv_msg);
      memset(recv_msg, 0, 256);

      send_ecu_msg("01 0A\r"); /* Fuel Pressure */
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("FP: %s\n", recv_msg);
      memset(recv_msg, 0, 256);

      send_ecu_msg("01 0B\r"); /* MAP Pressure */
      nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
      recv_ecu_msg(recv_msg);
      printf("MAP: %s\n", recv_msg);
      memset(recv_msg, 0, 256);

      send_ecu_msg("01 5E\r"); /* Fuel Flow Rate */  
      nanosleep(&reqtime, NULL); /* Sleep for 1 Second. */
      recv_ecu_msg(recv_msg);
      printf("FFR: %s\n", recv_msg);
      memset(recv_msg, 0, 256);

      send_ecu_msg("01 05\r"); /* Coolant Temperature */
      nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
      recv_ecu_msg(recv_msg);
      printf("ECT: %s\n", recv_msg);
      memset(recv_msg, 0, 256);

      send_ecu_msg("01 2F\r"); /* Fuel Tank Level */
      nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
      recv_ecu_msg(recv_msg);
      printf("FTL: %s\n", recv_msg);
      memset(recv_msg, 0, 256);

      send_ecu_msg("01 0F\r"); /* Intake Air Temperature */
      nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
      recv_ecu_msg(recv_msg);
      printf("IAT: %s\n", recv_msg);
      memset(recv_msg, 0, 256);

      send_ecu_msg("01 5C\r"); /* Oil Temperature */    
      nanosleep(&reqtime, NULL); /* Sleep for 100 milliSecond. */
      recv_ecu_msg(recv_msg);
      printf("OILT: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      
      send_ecu_msg("02 02\r"); /* Freeze Frame DTC. */
      nanosleep(&reqtime, NULL); 
      recv_ecu_msg(recv_msg);
      printf("Freeze Frame DTC: %s\n", recv_msg);
      memset(recv_msg, 0, 256);
      
      
      for (ii = 0; ii < 64; ii++)
      {
         /* TODO: send a bunch of supported PID request messages to non-standard mode numbers. */
         memset(protocol_req, 0, 256);
         sprintf(protocol_req, "%.2x 00\r", ii);
         printf("Sending supported PID request: %s\n", protocol_req);
         send_ecu_msg(protocol_req);
         nanosleep(&reqtime, NULL);
         while (recv_ecu_msg(recv_msg) > 0)
         {
            printf("ECU: <%d> %s\n", ii, recv_msg);
            memset(recv_msg, 0, 256);
         }
      }      
      
   }
   else
   {
      printf("Connection to ECU failed.\n");
      exit(-1);
   }

   printf("Server Test Exiting...\n");
   
   return(0);
}

