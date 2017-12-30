/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: OBD and AT message crackers. 
                If the message starts with ASCII hex digits '40'...'49' then
                the message is from the ECU. Otherwise it is a message from
                the OBD interface IC, is invalid or may be a manufacturer
                proprietary code.
                
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
    
    
    
   Date: 7/11/2017
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obd_monitor.h"
#include "protocols.h"

/* OBD Interface Parameters. */
OBD_Interface obd_interface;

/* ECU Parameters. */
ECU_Parameters ecup;

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

/*
   DTC Code Format: Each DTC is two bytes (two ASCII Hex characters).
   
   DTC System (Bits A7-A6): 00 = Powertrain, 01 = Chassis, 10 = Body, 11 = User(network).
   DTC Code Type (Bits A5-A4): 00 = Generic, 01 = Manufacturer, 10 and 11 = Mixed.
   DTC Number (Bits A3 - A0 and B7 - B0): numeric (hex) value DTC identifier.
*/

const char *DTC_System_Codes[16] = {
"P0","P1","P2","P3", /* Powertrain DTCs. */
"C0","C1","C2","C3", /* Chassis DTCs. */
"B0","B1","B2","B3", /* Body DTCs. */
"U0","U1","U2","U3", /* User/Network DTCs. */
};

const char *DTC_Type_Codes[4] = {
"Generic",
"Manufacturer",
"Generic/Manufacturer",
"Generic/Manufacturer"
};

PID_Parameters pid_list_mode_1[32];
PID_Parameters pid_list_mode_9[32];

DTC_Parameters dtc_list[32];

#define DTC_SYSTEM_CODE 0b11000000
#define DTC_TYPE_CODE   0b00110000
#define DTC_FIRST_NUM   0b00001111
#define DTC_SECOND_NUM  0b11110000
#define DTC_THIRD_NUM   0b00001111

/*
   DTC Count and MIL Status Message.
   MIL ON/OFF = Bit A7
   DTC Count = Bits A6-A0
*/
#define MIL_STATUS_BIT 128


/* OBD Interface Parameters Get/Set Functions. */ 
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

void set_obd_protocol_number(int obdpnum)
{
   obd_interface.obd_protocol_number = obdpnum;
   
   return;
}

int get_obd_protocol_number()
{
   return(obd_interface.obd_protocol_number);
}

/* ECU Parameters set/get functions. */

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
      ecup.ecu_engine_rpm = ((256.0 * (double)pa + (double)pb) / 4.0);
      /* ECU rpm parameter is in quarters of a revolution. */
      print_log_entry(rpm_msg);
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
      ecup.ecu_coolant_temperature = ((double)pa - 40.0);
      print_log_entry(ctemp_msg);
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
      ecup.ecu_manifold_air_pressure = (double)pa;
      print_log_entry(manap_msg);
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
      ecup.ecu_intake_air_temperature = ((double)pa - 40.0);
      print_log_entry(atemp_msg);
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
      ecup.ecu_battery_voltage = (double)bv;
      print_log_entry(bv_msg);
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
   char temp_buf[256];
   char *pch;
   
   memset(obd_interface.obd_interface_name, 0, 256);
   memset(temp_buf, 0, 256);

   if (strncmp(ii_msg, "ATI ", 4) == 0)
   {
      xstrcpy(obd_interface.obd_interface_name, ii_msg, 4, strlen(ii_msg)-2);
      print_log_entry(ii_msg);
   }
   else
   {
      strcpy(obd_interface.obd_interface_name, "Unknown OBD Interface");
   }
   strcpy(temp_buf, "Interface Type: ");
   strncat(temp_buf, obd_interface.obd_interface_name, strlen(obd_interface.obd_interface_name));
   set_status_bar_msg(temp_buf);
   
   return;
}

void get_interface_information(char *info)
{
   strncpy(info, obd_interface.obd_interface_name, strlen(obd_interface.obd_interface_name));
   
   return;
}

void set_obd_protocol_name(char *obd_protocol)
{
   char temp_buf[256];
   char *pch;
   
   memset(obd_interface.obd_protocol_name, 0, 256);
   memset(temp_buf, 0, 256);
   
   if (strncmp(obd_protocol, "ATDP ", 5) == 0)
   {
      xstrcpy(obd_interface.obd_protocol_name, obd_protocol, 5, strlen(obd_protocol)-2);
      print_log_entry(obd_protocol);
   }
   else if (strncmp(obd_protocol, "ATTP ", 5) == 0)
   {
      /* TODO: set protocol number from list, see simulator code. */
      xstrcpy(obd_interface.obd_protocol_name, obd_protocol, 5, strlen(obd_protocol)-2);
      print_log_entry(obd_protocol);
   }
   else if (strncmp(obd_protocol, "ATSP ", 5) == 0)
   {
      /* TODO: set protocol number from list, see simulator code. */

      xstrcpy(obd_interface.obd_protocol_name, obd_protocol, 5, strlen(obd_protocol)-2);
      print_log_entry(obd_protocol);
 
   }
   else
   {
      strcpy(obd_interface.obd_protocol_name, "Unknown OBD protocol.");
   }
   strcpy(temp_buf, "Protocol: ");
   strncat(temp_buf, obd_interface.obd_protocol_name, strlen(obd_interface.obd_protocol_name));
   printf("set_obd_protocol_name() <DEBUG>: %s\n", temp_buf);
   
   set_status_bar_msg(temp_buf);
   
   return;
}

void get_obd_protocol_name(char *info)
{
   strncpy(info, obd_interface.obd_protocol_name, strlen(obd_interface.obd_protocol_name));
   
   return;
}


void set_vehicle_speed(char *vs_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(vs_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      ecup.ecu_vehicle_speed = (double)pa;
      print_log_entry(vs_msg);
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
      ecup.ecu_throttle_position = 0.392 * (double)pa;
      print_log_entry(tp_msg);
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
      ecup.ecu_oil_temperature = ((double)pa - 40.0);
      print_log_entry(otemp_msg);
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
   return;
}

double get_oil_pressure()
{
   return(0);
}

void set_timing_advance(char *tadv_msg)
{
   unsigned int pmode, pid, pa;

   if (sscanf(tadv_msg, "%x %x %x", &pmode, &pid, &pa) == 3)
   {
      ecup.ecu_timing_advance = ((double)pa / 2.0) - 64.0;
      print_log_entry(tadv_msg);
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
      ecup.ecu_fuel_tank_level = ((float)pa * 0.392);
      print_log_entry(ftl_msg);
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
      ecup.ecu_fuel_flow_rate = ((256.0 * (double)pa) + (double)pb) / 20.0;
      print_log_entry(ffr_msg);
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
      ecup.ecu_fuel_pressure = (double)pa * 3.0; 
      print_log_entry(fp_msg);
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
      ecup.ecu_accelerator_position = 0.392 * (double)pa;
      print_log_entry(ap_msg);
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

void set_mode_1_supported_pid_list_1_32(char *pid_msg)
{
   unsigned int pmode, pid, pa, pb, pc, pd;
   unsigned int ii;
   unsigned long bit_select = 0x80000000;
   unsigned long bit_list;
   char temp_buf[256];

   if (sscanf(pid_msg, "%x %x %x %x %x %x", &pmode, &pid, &pa, &pb, &pc, &pd) == 6)
   {
      bit_list = (16777216 * pa) + (65536 * pb) + (256 * pc) + pd;
      printf("set_mode_1_supported_pid_list_1_32(): PID list = %.2x %.2x %.2x %.2x = %lx\n", pa, pb, pc, pd, bit_list);
      for (ii = 0; ii < 32; ii++)
      {
         if (bit_list & bit_select)
         {
            /* TODO: add to supported PID list. */
            sprintf(temp_buf, "set_mode_1_supported_pid_list_1_32(): PID %.2x supported.\n", ii+1);
            update_comms_log_view(temp_buf);
         }
         else
         {
            sprintf(temp_buf, "set_mode_1_supported_pid_list_1_32(): PID %.2x NOT supported.\n", ii+1);
            update_comms_log_view(temp_buf);
         }
         bit_select = bit_select >> 1;
      }
      print_log_entry(pid_msg);
   }
   else
   {
      printf("set_mode_1_supported_pid_list_1_32() <ERROR>: Invalid message.\n");
   }
   

   return;
}

double get_mode_1_supported_pid_list_1_32()
{
   /* TODO: */
   return(0);
}


void set_mode_9_supported_pid_list_1_32(char *pid_msg)
{
   unsigned int pmode, pid, pa, pb, pc, pd;
   unsigned int ii;
   unsigned long bit_select = 0x80000000;
   unsigned long bit_list;
   char temp_buf[256];

   if (sscanf(pid_msg, "%x %x %x %x %x %x", &pmode, &pid, &pa, &pb, &pc, &pd) == 6)
   {
      bit_list = (16777216 * pa) + (65536 * pb) + (256 * pc) + pd;
      printf("set_mode_9_supported_pid_list_1_32(): PID list = %.2x %.2x %.2x %.2x = %lx\n", pa, pb, pc, pd, bit_list);
      for (ii = 0; ii < 32; ii++)
      {
         if (bit_list & bit_select)
         {
            /* TODO: add to supported PID list. */
            sprintf(temp_buf, "set_mode_9_supported_pid_list_1_32(): PID %.2x supported.\n", ii+1);
            update_comms_log_view(temp_buf);
         }
         else
         {
            sprintf(temp_buf, "set_mode_9_supported_pid_list_1_32(): PID %.2x NOT supported.\n", ii+1);
            update_comms_log_view(temp_buf);
         }
         bit_select = bit_select >> 1;
      }
      print_log_entry(pid_msg);
   }
   else
   {
      printf("set_mode_9_supported_pid_list_1_32() <ERROR>: Invalid message.\n");
   }
   
   return;
}

double get_mode_9_supported_pid_list_1_32()
{
   return(0);
}

void set_vehicle_vin(char *obd_vin_msg)
{
   char temp_buf[256];
   int len;
   memset(ecup.ecu_vin, 0, 256);
   memset(temp_buf, 0, 256);
   
   len = xstrcpy(temp_buf, obd_vin_msg, 6, strlen(obd_vin_msg)-2); /* Chomp the header and newline. */
   if (len > 0)
   {
      len = xhextoascii(ecup.ecu_vin, temp_buf);
      if (len > 0)
      {
         print_log_entry(obd_vin_msg);
      }
   }
   else
   {
      strcpy(ecup.ecu_vin, "Invalid VIN Message.");
   }
   strcpy(temp_buf, "VIN: ");
   strncat(temp_buf, ecup.ecu_vin, strlen(ecup.ecu_vin));
   set_status_bar_msg(temp_buf);
   
   return;
}

void set_ecu_name(char *obd_ecu_name_msg)
{
   char temp_buf[256];
   int len;
   memset(ecup.ecu_name, 0, 256);
   memset(temp_buf, 0, 256);
   
   len = xstrcpy(temp_buf, obd_ecu_name_msg, 6, strlen(obd_ecu_name_msg)-2); /* Chomp the header and newline. */
   if (len > 0)
   {
      len = xhextoascii(ecup.ecu_name, temp_buf);
      strcpy(temp_buf, "ECU Name: ");
      strncat(temp_buf, ecup.ecu_name, strlen(ecup.ecu_name));
      set_status_bar_msg(temp_buf);
      print_log_entry(obd_ecu_name_msg);
   }
   else
   {
      strcpy(ecup.ecu_name, "Invalid ECU Name Message.");
      printf(temp_buf, "Invalid ECU Name Message: %s\n", obd_ecu_name_msg);
   }
   
   return;
}

int get_mil_status()
{
   return(ecup.ecu_mil_status);
}

int get_dtc_count()
{
   return(ecup.ecu_dtc_count);
}

void set_dtc_count(char *obd_mil_msg)
{
   int len;
   long mil_code;
   char buf[256];
   char mil_chars[2];
   
   len = strlen(obd_mil_msg);
   
   if (len > 8)
   {
     mil_chars[0] = obd_mil_msg[6];
     mil_chars[1] = obd_mil_msg[7];
     mil_code = strtol(mil_chars, 0, 16);
     if (mil_code >= 128)
     {
        /* MIL is on. */
        ecup.ecu_mil_status = 1;
        ecup.ecu_dtc_count = mil_code - 128;
        printf(buf, "MIL On: DTC Count = %d", ecup.ecu_dtc_count);
        set_status_bar_msg(buf);
     }
     else
     {
        /* MIL is off. */
        ecup.ecu_mil_status = 0;
        ecup.ecu_dtc_count = mil_code;
        printf("MIL Off: DTC Count = %d", ecup.ecu_dtc_count);
     }
     print_log_entry(obd_mil_msg);
   }
   
   return;
}

void get_last_dtc_code(char *code_buf)
{
   int n;
   
   n = strlen(ecup.ecu_last_dtc_code);
   if (n > 0)
     strncpy(code_buf, ecup.ecu_last_dtc_code, n);
   else
     strncpy(code_buf, "00000", 5);
   
   return;
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
         case 0: set_mode_1_supported_pid_list_1_32(obd_msg); break; /* TODO: Supported PIDs. */
         case 1: set_dtc_count(obd_msg);
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
      printf("Invalid OBD Mode 01 Message: %s\n", obd_msg);
   }   

   
   return;
}

void parse_mode_03_msg(char *obd_dtc_msg)
{
   /* Decode DTC message. */
   /* 
      Message format Non-CAN (7 hex values): 43 01 33 00 00 00 00
      TODO: Message format CAN (8 hex values): 43 01 01 33 00 00 00 00
   */
   int len;
   int dtc_index;
   char buf[256];
   char dtc_sys_chars[4];
   char dtc_code[8];
   
   len = strlen(obd_dtc_msg);
   /* printf("parse_mode_03_msg() <DEBUG>: %s\n", obd_msg); */
   if (len > 8)
   {
      memset(dtc_code, 0, 8);
      dtc_code[0] = obd_dtc_msg[3];
      dtc_sys_chars[0] = obd_dtc_msg[3];
      dtc_sys_chars[1] = obd_dtc_msg[4];
      dtc_sys_chars[2] = obd_dtc_msg[6];
      dtc_sys_chars[3] = obd_dtc_msg[7];
      dtc_sys_chars[4] = 0;
      dtc_index = strtol(dtc_code, 0, 16);
      /* printf("parse_mode_03_msg() <DEBUG>: %d %s %s\n", dtc_index, dtc_code, dtc_sys_chars); */
      if ((dtc_index >= 0) && (dtc_index < 16)) /* TODO: handle multiple DTCs. */
      {
         strncpy(ecup.ecu_last_dtc_code, DTC_System_Codes[dtc_index], 2);
         ecup.ecu_last_dtc_code[2] = dtc_sys_chars[1];
         ecup.ecu_last_dtc_code[3] = dtc_sys_chars[2];
         ecup.ecu_last_dtc_code[4] = dtc_sys_chars[3];
         sprintf(buf, "Diagnostic Trouble Code: %s", ecup.ecu_last_dtc_code);
         set_status_bar_msg(buf);
         print_log_entry(obd_dtc_msg);
         /* printf("parse_mode_03_msg() <INFO>: %s\n", buf); */
      }
      else
      {
         printf("parse_mode_03_msg() <ERROR>: %s\n", obd_dtc_msg);
      }
   }
   else
   {
      printf("parse_mode_03_msg() <ERROR>: %s\n", obd_dtc_msg);
   }
     
   return;
}

void parse_mode_09_msg(char *obd_msg)
{
   /* Decode ECU Mode 09 parameter message. */
   unsigned int pmode, pid;
   char header[16];
   
   memset(header, 0, 16);
   strncpy(header, obd_msg, 5);
   
   if (sscanf(header, "%x %x", &pmode, &pid) == 2)
   {
      switch(pid)
      {
         case 0: set_mode_9_supported_pid_list_1_32(obd_msg); break; /* TODO: Supported PIDs. */
         case 2: set_vehicle_vin(obd_msg); break;
         case 10: set_ecu_name(obd_msg); break;
      }
   }
   return;
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
            /* TODO: Process information from OBD interface. */
            if (strncmp(obd_msg, "ATRV", 4) == 0)
            {
               set_battery_voltage(obd_msg);
               result = 2; 
            }
            else if (strncmp(obd_msg, "ATI", 3) == 0)
            {
               set_interface_information(obd_msg);
               result = 3; 
            }
            else if (strncmp(obd_msg, "ATDP", 4) == 0)
            {
               set_obd_protocol_name(obd_msg);
               result = 4; 
            }
            else if (strncmp(obd_msg, "ATSP", 4) == 0)
            {
               set_obd_protocol_name(obd_msg);
               result = 4; 
            }
            else if (strncmp(obd_msg, "ATTP", 4) == 0)
            {
               set_obd_protocol_name(obd_msg);
               result = 4; 
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

