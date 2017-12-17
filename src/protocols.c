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
OBD_Interface obd_interface;

/* ECU Parameters. */
ECU_Parameters ecup;

void set_interface_on()
{
   /* Interface is ready to receive messages. */
   obd_interface.obd_interface_status = 1;
   return;
}

void set_interface_off()
{
   /* Interface is not ready to receive messages, may be busy or powered off. */
   obd_interface.obd_interface_status = 0;
   return;
}

int get_interface_status()
{
   return(obd_interface.obd_interface_status);
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
   return(ecup.ecu_engine_rpm);
}

void set_coolant_temperature(char *ctemp_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(ctemp_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("ECT Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_coolant_temperature = ((double)pa - 40.0);
   }
   else
   {
      ecup.ecu_coolant_temperature = 0.0;
   }
   return;
}

double get_coolant_temperature()
{
   return(ecup.ecu_coolant_temperature);
}

void set_manifold_pressure(char *manap_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(manap_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("MAP Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_manifold_air_pressure = (double)pa;
   }
   else
   {
      ecup.ecu_manifold_air_pressure = 0.0;
   }
   
   return;
}

double get_manifold_pressure()
{
   return(ecup.ecu_manifold_air_pressure);
}

void set_intake_air_temperature(char *atemp_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(atemp_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("IAT Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_intake_air_temperature = ((double)pa - 40.0);
   }
   else
   {
      ecup.ecu_intake_air_temperature = 0.0;
   }
   
   return;
}

double get_intake_air_temperature()
{
   return(ecup.ecu_intake_air_temperature);
}

void set_battery_voltage(char *bv_msg)
{
   float bv;

   if (sscanf(bv_msg, "ATRV %f", &bv) == 1)
   {
      printf("Battery Voltage Msg: %.2f\n", bv); 
      ecup.ecu_battery_voltage = (double)bv;
      /* sscanf(bv_msg, "ATRV %s\n", ecup.battery_voltage); */
   }
   else
   {
      ecup.ecu_battery_voltage = 0.0;
   }
   
   return;
}

double get_battery_voltage()
{
   return(ecup.ecu_battery_voltage);
}

void set_interface_information(char *ii_msg)
{
   memset(obd_interface.obd_interface_name, 0, 256);

   if (sscanf(ii_msg, "ATI %s", obd_interface.obd_interface_name) == 1)
   {
      printf("ATI Msg: %s", ii_msg); 
   }
   else
   {
      strncpy(obd_interface.obd_interface_name, "Unknown OBD Interface", 21);
   }
   
   return;
}

void get_interface_information(char *info)
{
   strncpy(info, obd_interface.obd_interface_name, strlen(obd_interface.obd_interface_name));
   return;
}

void set_vehicle_speed(char *vs_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(vs_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("Vehicle Speed Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_vehicle_speed = (double)pa;
   }
   else
   {
      ecup.ecu_vehicle_speed = 0.0;
   }
   
   return;
}

double get_vehicle_speed()
{
   return(ecup.ecu_vehicle_speed);
}

void set_egr_pressure(char *egrp_msg)
{

}

double get_egr_pressure()
{

}

void set_throttle_position(char *tp_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(tp_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("Throttle Position Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_throttle_position = 0.392 * (double)pa;
   }
   else
   {
      ecup.ecu_throttle_position = 0.0;
   }
   
   return;
}

double get_throttle_position()
{
   return(ecup.ecu_throttle_position);
}

void set_oil_temperature(char *otemp_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(otemp_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("Oil Temperature Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_oil_temperature = ((double)pa - 40.0);
   }
   else
   {
      ecup.ecu_oil_temperature = 0.0;
   }
   return;
}

double get_oil_temperature()
{
   return(ecup.ecu_oil_temperature);
}

void set_oil_pressure(char *oilp_msg)
{

}

double get_oil_pressure()
{

}

void set_supported_pid_list_1_32(char *pid_msg)
{

}

double gset_supported_pid_list_1_32()
{

}

void set_timing_advance(char *tadv_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(tadv_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("Timing Advance Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_timing_advance = (double)pa / 2.0 - 64.0;
   }
   else
   {
      ecup.ecu_timing_advance = 0.0;
   }
   
   return;
}

double get_timing_advance()
{
   return(ecup.ecu_timing_advance);
}


void set_fuel_tank_level(char *ftl_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(ftl_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("Fuel Tank Level Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_fuel_tank_level = 0.392 * (double)pa;
   }
   else
   {
      ecup.ecu_fuel_tank_level = 0.0;
   }
   
   return;
}

double get_fuel_tank_level()
{
   return(ecup.ecu_fuel_tank_level);
}

void set_fuel_flow_rate(char *ffr_msg)
{
   unsigned int pmode, pid, pa, pb;

   if (sscanf(ffr_msg, "%x %x %x %x", &pmode, &pid, &pa, &pb) == 4)
   {
      printf("Fuel Flow Rate Msg: %d %d %d %d\n", pmode, pid, pa, pb); 
      ecup.ecu_fuel_flow_rate = ((256.0 * (double)pa) + (double)pb) / 20.0;
   }
   else
   {
      ecup.ecu_fuel_flow_rate = 0.0;
   }
   
   return;
}

double get_fuel_flow_rate()
{
   return(ecup.ecu_fuel_flow_rate);
}


void set_fuel_pressure(char *fp_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(fp_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("Fuel Pressure Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_fuel_pressure = 3.0 * (double)pa;
   }
   else
   {
      ecup.ecu_fuel_pressure = 0.0;
   }
   
   return;
}

double get_fuel_pressure()
{
   return(ecup.ecu_fuel_pressure);
}


void set_accelerator_position(char *ap_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(ap_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      printf("Accelerator Position Msg: %d %d %d\n", pmode, pid, pa); 
      ecup.ecu_accelerator_position = 0.392 * (double)pa;
   }
   else
   {
      ecup.ecu_accelerator_position = 0.0;
   }
   
   return;
}

double get_accelerator_position()
{
   return(ecup.ecu_accelerator_position);
}


void parse_mode_01_msg(char *obd_msg)
{
   /* Decode ECU Mode 01 parameter message. */
   unsigned int pmode, pid;
   char header[16];
   
   memset(header, 0, 16);
   strncpy(header, obd_msg, 5);
   
   if (sscanf(header, "%x %x", &pmode, &pid) == 2)
   {
      switch(pid)
      {
         case 0: set_supported_pid_list_1_32(obd_msg); break; /* TODO: Supported PIDs. */
         case 5: set_coolant_temperature(obd_msg); break; /*  */
         case 10: set_fuel_pressure(obd_msg); break;
         case 11: set_manifold_pressure(obd_msg); break; /* Throttle Position. */
         case 12: set_engine_rpm(obd_msg); break;
         case 13: set_vehicle_speed(obd_msg); break;
         case 14: set_timing_advance(obd_msg); break;
         case 15: set_intake_air_temperature(obd_msg); break;
         case 17: set_throttle_position(obd_msg); break;
         case 47: set_fuel_tank_level(obd_msg); break; /* Fuel Tank Level. */
         case 90: set_accelerator_position(obd_msg); break;
         case 92: set_oil_temperature(obd_msg); break;
         case 94: set_fuel_flow_rate(obd_msg); break;
         default : break; /* TODO: process unknown PID. */
      }

   }
   else
   {
      /* TODO: log error message. */
   }   

   
   return;
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
   int msg_len, result;
   char temp_buf[256];
   
   msg_len = strlen(obd_msg);
   
   if ((msg_len > 0) && (obd_msg[msg_len - 1] == '\n')) /* All messages must terminate with a newline. */
   {
      /* Parse the message. */
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
            default : break; /* TODO: process unknown mode. */
         }
         result = 0;
      }
      else /* This is an AT message response from the OBD interface. */
      {
         /* TODO: Process AT message and save configuration info from the interface. */
         if (obd_msg[0] == '>') /* ELM327 IC sends a '>' character to signal it is ready. */
         {
            obd_interface.obd_interface_status = 1; /* Interface is ready to receive messages. */
            result = 1;
         }
         else
         {
            result = 2; /* TODO: Process information from OBD interface. */
            if (strncmp(obd_msg, "ATRV", 4) == 0)
            {
               set_battery_voltage(obd_msg);
            }
            if (strncmp(obd_msg, "ATI", 3) == 0)
            {
               set_interface_information(obd_msg);
            }
         }
      }
   }
   else
   {
      /* TODO: Invalid message, log an error. */
      result = -1;
   }

   return(result);
}

