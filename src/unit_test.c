/* 

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
#include "pid_hash_map.h"
#include "dtc_hash_map.h"

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

/* Example VINs, first is non CAN protocol, second is CAN protocol
   for the same vehicle identification number. */
const char *ecu_vin[] = { 
"30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A",
"49 02 31 44 34 47 50 30 30 52 35 35 42 31 32 33 34 35 36",
"49 02 01 31 44 34 47 50 30 30 52 35 35 42 31 32 33 34 35 36"
};

const char *ecu_name[] = { 
"49 0A 43 52 41 50 54 45 43 48 3A 53 59 53 54 45 4D 53 3A 30 31 32 33 34",
"49 0A 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46 47 48 49 4A 4B 4C",
"49 0A 01 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A 31 32 33 34 35 36 37 38"
};


const char *test_strings[] = { 
"this is test STRING one.",
"THIS is test STRING TWO.",
"1234.....4321.....898911",
"127.0.0.1",
"999.999.999.999",
"255.255.255.255",
"01 A1 B0 30 08 01",
"41 00 K0 04 C9 FF",
"ATZ!ELM327 v2.1!"
};

void generate_dtc_lookup_table()
{
   return;
}


int main(int argc, char *argv[])
{
   char temp_buf[256];
   char obd_msg[256];
   int len, ii;
   
   memset(temp_buf, 0, 256);
   memset(obd_msg, 0, 256);
   
   if (argc < 2) 
   {
      /* fprintf(stderr, "main() <ERROR>: no port provided.\n");
      exit(0); */
   }
   
   if (open_log_file("./", "unit_tests_log.txt") == -1)
   {
      printf("unit_test <ERROR>: Could not open log file.\n");
   }

/* 
----------------------------------------------
         Generate DTC Lookup Table. 
----------------------------------------------
*/
   /* generate_dtc_lookup_table(); */

/* 
----------------------------------------------
         Function tests util.c 
----------------------------------------------
*/
   for (ii = 0; ii < 3; ii++)
   {
      strcpy(obd_msg, ecu_vin[ii]);
      
      len = xhextoascii(temp_buf, obd_msg);
      if (len > 0)
      {
         print_log_entry(temp_buf);
      }
      else
      {
         strcpy(temp_buf, "Unknown VIN Message.");
      }
      printf("VIN: %s\n", temp_buf);
   }

   for (ii = 0; ii < 3; ii++)
   {
      strcpy(obd_msg, ecu_name[ii]);
      
      len = xhextoascii(temp_buf, obd_msg);
      if (len > 0)
      {
         print_log_entry(temp_buf);
      }
      else
      {
         strcpy(temp_buf, "Unknown ECU Message.");
      }
      printf("ECU: %s\n", temp_buf);
   }

   for (ii = 0; ii < 2; ii++)
   {
      strcpy(temp_buf, test_strings[ii]);
      
      uppercase(temp_buf);

      print_log_entry(temp_buf);

      printf("uppercase(): %s\n", temp_buf);
   }
   
   for (ii = 0; ii < 9; ii++)
   {
      strcpy(temp_buf, test_strings[ii]);
      
      replacechar(temp_buf, '.', ' ');

      print_log_entry(temp_buf);

      printf("replacechar(): %s\n", temp_buf);
   }
         
/* 
----------------------------------------------
         Hashmap tests.
----------------------------------------------
*/

   for(ii = 0; ii < 10; ii++)
   {
         PID_Parameters *pid = (PID_Parameters *) xmalloc(sizeof(PID_Parameters));
         sprintf(pid->pid_code, "%.4x", ii);
         sprintf(pid->pid_description, "MODE 01: %.4x", ii);
         add_pid(pid); 
   }
   print_pid_map();

   for(ii = 0; ii < 10; ii++)
   {
         DTC_Parameters *dtc = (DTC_Parameters *) xmalloc(sizeof(DTC_Parameters));
         sprintf(dtc->dtc_code, "%.4x", ii);
         sprintf(dtc->dtc_description, "Powertrain: %.4x", ii);
         add_dtc(dtc); 
   }
   print_dtc_map();
   
   
   close_log_file();
   
   exit(0);
}
