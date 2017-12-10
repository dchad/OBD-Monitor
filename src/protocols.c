/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: OBD and AT message crackers. 
                If the message starts with ASCII hex digits '40'...'49' then
                the message is from the ECU. Otherwise it is a message from
                the OBD interface IC or is invalid.

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

void set_ecu_parameters(ECU_Parameters *ecup)
{
   /* TODO: */
}

void get_ecu_parameters(ECU_Parameters *ecup)
{
   /* TODO: */
}

void set_engine_rpm(double rpm)
{
   ecup.ecu_engine_rpm = rpm / 4.0; /* ECU rpm parameter is in quarters of a revolution. */
   return;
}

double get_engine_rpm()
{

   return ecup.ecu_engine_rpm;
}

void set_coolant_temperature(double ctemp)
{

}

double get_coolant_temperature()
{

}

void set_manifold_pressure(double manap)
{

}

double get_manifold_pressure()
{

}

void set_intake_air_temperature(double atemp)
{

}

double get_intake_air_temperature()
{

}

void set_battery_voltage(double bv)
{

}

double get_battery_voltage()
{

}

void set_vehicle_speed(double vs)
{

}

double get_vehicle_speed()
{

}

void set_egr_pressure(double egrp)
{

}

double get_egr_pressure()
{

}

void set_oil_temperature(double otemp)
{

}

double get_oil_temperature()
{

}

void set_oil_pressure(double oilp)
{

}

double get_oil_pressure()
{

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
               case '0': break;
               case '1': break;
               case '2': break;
               case '3': break;
               case '4': break;
               case '5': break;
               case '6': break;
               case '7': break;
               case '8': break;
               case '9': break;
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
      /* TODO: Invalid message, pop up an error dialog. */
   }


}

