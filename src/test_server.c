/* 
   Project: OBD-II Monitor (On-Board Diagnostics)
   
   
   File: test_server.c
   
   
   Author: Derek Chadwick

   Description: A test set for the UDP server that communicates with vehicle
                engine control units via an OBD-II interface to obtain 
                engine status and fault codes. Can also be used as an ECU
                communications logging function if the GUI is not required.

                Tests two functions:

                1. The UDP datagram server that receives requests for vehicle
                status information from a client application (GUI) and 
                returns the requested information to the client. 

                2. Serial communications to request vehicle status
                information and fault codes from the engine control unit using 
                the OBD-II protocol.
                

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

/* 
void set_status_bar_msg(char *msg)
{
   
   return;
}

void update_comms_log_view(char *msg)
{
   
   return;
}
*/

int main()
{
   char recv_msg[256];
   /* */
   set_ecu_connected(0);
   int ecu_auto_connect = 1; /* TODO: add to configuration options: get_auto_connect(). */
   if (ecu_auto_connect == 1) 
   {
      if (ecu_connect(recv_msg) > 0) /* Sockets Module Connect Function. */
      {
         send_ecu_msg("ATDP\n");  /* Get OBD protocol name from interface. */
         send_ecu_msg("ATRV\n");  /* Get battery voltage from interface. */
         send_ecu_msg("09 02\n"); /* Get vehicle VIN number. */
         send_ecu_msg("09 0A\n"); /* Get ECU name. */
         send_ecu_msg("01 01\n"); /* Get DTC Count and MIL status. */
         send_ecu_msg("01 00\n"); /* Get supported PIDs 1 - 32 for MODE 1. */
         send_ecu_msg("09 00\n"); /* Get supported PIDs 1 - 32 for MODE 9. */
         send_ecu_msg("03\n");
         /* TODO: timeout callbacks and recv looop.
         g_timeout_add (60000, send_obd_message_60sec_callback, (gpointer)window);
         g_timeout_add (10000, send_obd_message_10sec_callback, (gpointer)window);
         g_timeout_add (1000, send_obd_message_1sec_callback, (gpointer)window);
         g_timeout_add (100, recv_obd_message_callback, (gpointer)window); */
         printf("Connected to ECU.");
      }
      else
      {
         printf("Connection to ECU failed.");
      }
   }   
   
   return(0);
}

