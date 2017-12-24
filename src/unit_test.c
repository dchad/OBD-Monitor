

#include "obd_monitor.h"

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

int main(int argc, char *argv[])
{
   char temp_buf[256];
   char obd_msg[256];
   int len;
   memset(temp_buf, 0, 256);
   memset(obd_msg, 0, 256);
   
   if (argc < 2) 
   {
      /* fprintf(stderr, "main() <ERROR>: no port provided.\n");
      exit(0); */
   }


/* 
----------------------------------------------
         Function tests util.c 
----------------------------------------------
*/
   strcpy(obd_msg, ecu_vin[0]);
   
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
   
      
   
   exit(0);
}
