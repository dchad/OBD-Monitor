/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: OBD and AT message crackers. 
                If the message starts with ASCII hex digits '40'...'49' then
                the message is from the ECU. Otherwise it is a message from
                the OBD interface IC or is invalid.
                
   Selected ECU Mode 01 Parameters: 
   
   [PID] [Data Bytes] [Min Value] [Max Value] [Formula]           [Description]
    05    1            -40         215         A - 40             (ECT Centigrade)
    0A    1            0           765         3 * A              (Fuel Pressure kPa)
    0B    1            0           255         A                  (MAP Pressure kPa)
    0C    2            0           16,383.75   (256 * A + B) / 4  (Engine RPM)
    0D    1            0           255         A                  (Vehicle Speed)
    0E    1            -64         63.5        (A / 2) - 64       (Timing Advance: degrees before TDC) 
    0F    1            -40         215         A - 40             (IAT Centigrade)
    11    1            0           100         100 / 255 * A      (Throttle Position %)
    2F    1            0           100         100 / 255 * A      (Fuel Tank Level %)
    5A    1            0           100         100 / 255 * A      (Relative Accelerator Pedal Position %)
    5C    1            -40         215         A - 40             (Oil Temperature)
    5E    2            0           3276.75     (256 * A + B) / 20 (Fuel Flow Rate L/h)



   Selected ECU Mode 09 Parameters:
 
   [PID] [Data Bytes] [Description] 
    02    17           VIN - Vehicle Identification Number
    0A    20           ECU Name
         
     
   Selected ECU Mode 21 Parameters (Toyota):
   
       
                     
   Selected ECU Mode 22 Parameters (GM/Isuzu):
        
    (Oil Pressure? GM = 22115c)
    
    (EGR Pressure?)
    
    
   Date: 7/11/2017
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocols.h"

/* OBD Interface Parameters. */
int interface_status;

/* ECU Parameters. */

ECU_Parameters ecup;

void set_interface_on()
{
   /* Interface is ready to receive messages. */
   interface_status = 1;
   return;
}

void set_interface_off()
{
   /* Interface is not ready to receive messages, may be busy or powered off. */
   interface_status = 0;
   return;
}

int get_interface_status()
{
   return interface_status;
}

void set_ecu_parameters(ECU_Parameters *ecupin)
{
   /* TODO: */
}

void get_ecu_parameters(ECU_Parameters *ecupout)
{
   /* TODO: */
}

void log_ecu_parameters()
{
   /* TODO: log ecu parameters on a 60 second timer. */
}

void set_engine_rpm(char *rpm_msg)
{
   char buf[16];
   char *cptr;
   unsigned int pmode, pid, pa, pb;

   if (sscanf(rpm_msg, "%x %x %x %x", &pmode, &pid, &pa, &pb) == 4)
   {
      printf("RPM Msg: %d %d %d %d\n", pmode, pid, pa, pb); 
      ecup.ecu_engine_rpm = ((256.0 * (double)pa + (double)pb) / 4.0);
      /* ECU rpm parameter is in quarters of a revolution. */
   }
   else
   {
      ecup.ecu_engine_rpm = 0.0;
   }
   return;
}

double get_engine_rpm()
{

   return ecup.ecu_engine_rpm;
}

void set_coolant_temperature(char *ctemp_msg)
{

}

double get_coolant_temperature()
{

}

void set_manifold_pressure(char *manap_msg)
{

}

double get_manifold_pressure()
{

}

void set_intake_air_temperature(char *atemp_msg)
{

}

double get_intake_air_temperature()
{

}

void set_battery_voltage(char *bv_msg)
{

}

double get_battery_voltage()
{

}

void set_vehicle_speed(char *vs_msg)
{

}

double get_vehicle_speed()
{

}

void set_egr_pressure(char *egrp_msg)
{

}

double get_egr_pressure()
{

}

void set_oil_temperature(char *otemp_msg)
{

}

double get_oil_temperature()
{

}

void set_oil_pressure(char *oilp_msg)
{

}

double get_oil_pressure()
{

}

void parse_mode_01_msg(char *obd_msg)
{
   /* Decode ECU parameter message. */
}

void parse_mode_03_msg(char *obd_msg)
{
   /* Decode DTC message. */
}

void parse_mode_09_msg(char *obd_msg)
{
   /* Decode ECU information message. */
}


int parse_obd_msg(char *obd_msg)
{
   int msg_len;
   char temp_buf[256];
   
   msg_len = strlen(obd_msg);
   
   if ((msg_len > 0) && (obd_msg[msg_len - 1] == '\n')) /* All messages must terminate with a newline. */
   {
      /* Parse the message. */
      if (obd_msg[0] == '>') /* ELM327 IC sends a '>' character to signal it is ready. */
      {
         interface_status = 1; /* Interface is ready to receive messages. */
      }
      else
      {
         if (obd_msg[0] == '4') /* This is an OBD response message from the ECU. */
         {
            switch(obd_msg[1])
            {
               case '0': break; /* Invalid mode. */
               case '1': parse_mode_01_msg(obd_msg); break; /* Mode 01 message, ECU parameter update. */
               case '2': break;
               case '3': parse_mode_03_msg(obd_msg); break; /* Mode 03 message, diagnostic trouble codes. */
               case '4': break;
               case '5': break;
               case '6': break;
               case '7': break;
               case '8': break;
               case '9': parse_mode_09_msg(obd_msg); break; /* Mode 09 message, ECU information. */
               case 'A': break;
            }
         }
         else /* This is an AT message response for the interface. */
         {
            /* TODO: Save configuration info from the interface. */
         }
      }
   
   }
   else
   {
      /* TODO: Invalid message, log an error. */
   }


}

