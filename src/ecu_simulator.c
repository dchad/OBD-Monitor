/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A UDP server that simulates an engine control unit
                and an ELM372 OBD interface for functional and unit
                testing without any hardware or vehicles.
                
   Selected ECU Mode 01 Parameters: 
   
   [PID] [Data Bytes] [Min Value] [Max Value] [Formula]           [Description]
    05    1            -40         215         A - 40             (ECT Centigrade)
    0B    1            0           255         A                  (MAP Pressure kPa)
    0C    2            0           16,383.75   (256 * A + B) / 4  (Engine RPM)
    0D    1            0           255         A                  (Vehicle Speed) 
    0F    1            -40         215         A - 40             (IAT Centigrade)
    11    1            0           100         100 / 256 * A      (Throttle Position %)
    5C    1            -40         215         A - 40             (Oil Temperature)
    5E    2            0           3276.75     (256 * A + B) / 20 (Fuel Flow Rate L/h)
    
    (Oil Pressure?)
    (EGR Pressure?)
    (Accelerator Position)

   Selected ECU Mode 09 Parameters:
 
   [PID] [Data Bytes] [Description] 
    02    17           VIN - Vehicle Identification Number
    0A    20           ECU Name
    
   
   Date: 30/11/2017
   
*/

#include "obd_monitor.h"
#include "protocols.h"
#include "rs232.h"


ECU_Parameters simulator_ecu;
OBD_Interface simulator_obd;

unsigned int tick_count;

int sock, length, n, serial_port;
socklen_t from_len;
struct sockaddr_in server;
struct sockaddr_in from_client;
char in_buf[BUFFER_MAX_LEN];
unsigned char ecu_msg[BUFFER_MAX_LEN];
   
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
   for the same vehicle identification number.
   Example: 1 D 4 G P 0 0 R 5 5 B 1 2 3 4 5 6   
*/
const char *ecu_vin[] = { 
"49 02 31 44 34 47 50 30 30 52 35 35 42 31 32 33 34 35 36",
"49 02 01 31 44 34 47 50 30 30 52 35 35 42 31 32 33 34 35 36"
};

const char *ecu_name[] = { 
"49 0A 43 52 41 50 54 45 43 48 3A 53 59 53 54 45 4D 53 3A 30 31 32 33 34",
"49 0A 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46 47 48 49 4A 4B 4C",
"49 0A 01 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A 31 32 33 34 35 36 37 38"
};


void fatal_error(const char *error_msg)
{
    perror(error_msg);
    exit(0);
}


void set_simulator_ecu_parameters()
{
   /* TODO: set all the ecu parameters. */
   simulator_ecu.ecu_engine_rpm = 10.0 * (double)tick_count;
   simulator_ecu.ecu_vehicle_speed = (double)tick_count;
   simulator_ecu.ecu_coolant_temperature = (double)tick_count;
   simulator_ecu.ecu_intake_air_temperature = 2.0 * (double)tick_count;
   simulator_ecu.ecu_manifold_air_pressure = 3.0 * (double)tick_count;
   /* simulator_ecu.ecu_oil_pressure = 5.0 * (double)tick_count;
   simulator_ecu.ecu_egr_pressure = 5.0 * (double)tick_count; */
   simulator_ecu.ecu_battery_voltage = 12.5;
   simulator_ecu.ecu_throttle_position = (double)tick_count;
   simulator_ecu.ecu_oil_temperature = 2.0 * (double)tick_count;
   simulator_ecu.ecu_accelerator_position = (double)tick_count;
   simulator_ecu.ecu_fuel_pressure = 3.0 * (double)tick_count;
   simulator_ecu.ecu_fuel_flow_rate = 2.0 * (double)tick_count;
   simulator_ecu.ecu_fuel_tank_level = (double)tick_count;
   
   simulator_ecu.ecu_mil_status = 1;
   simulator_ecu.ecu_dtc_count = 1;
    
   return;
}

void get_simulator_ecu_parameters(ECU_Parameters *ecupout)
{
   /* TODO: */
}

void log_simulator_ecu_parameters()
{
   /* TODO: log ecu parameters on a 60 second timer. */
}

int send_engine_rpm()
{
   char reply_buf[256];
   unsigned int rpm_temp, rpm_A, rpm_B;
   int n;
   
   memset(reply_buf, 0, 256);

   rpm_temp = (unsigned int)simulator_ecu.ecu_engine_rpm * 4;
   rpm_A = rpm_temp / 256;
   rpm_B = rpm_temp % 256;
   
   sprintf(reply_buf, "41 0C %.2x %.2x\n", rpm_A, rpm_B);
   /* TODO: log simulator msg. */
   printf("send_engine_rpm(): Simulator RPM Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}


int send_coolant_temperature()
{
   char reply_buf[256];
   unsigned int ect_A;
   int n;
   
   memset(reply_buf, 0, 256);

   ect_A = (unsigned int)simulator_ecu.ecu_coolant_temperature + 40;
   
   sprintf(reply_buf, "41 05 %.2x\n", ect_A);
   /* TODO: log simulator msg. */
   printf("send_coolant_temperature(): Simulator ECT Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}


void send_manifold_pressure()
{
   char reply_buf[256];
   unsigned int map_A;
   int n;
   
   memset(reply_buf, 0, 256);

   map_A = (unsigned int)simulator_ecu.ecu_manifold_air_pressure;
   
   sprintf(reply_buf, "41 0B %.2x\n", map_A);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   printf("send_manifold_pressure(): Simulator MAP Msg: %i bytes %s", n, reply_buf);
      
   return;
}


int send_intake_air_temperature()
{
   char reply_buf[256];
   unsigned int iat_A;
   int n;
   
   memset(reply_buf, 0, 256);

   iat_A = (unsigned int)simulator_ecu.ecu_intake_air_temperature + 40;
   
   sprintf(reply_buf, "41 0F %.2x\n", iat_A);
   /* TODO: log simulator msg. */
   printf("send_intake_air_temperature(): Simulator IAT Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}


int send_vehicle_speed()
{
   char reply_buf[256];
   unsigned int vs_A;
   int n;
   
   memset(reply_buf, 0, 256);

   vs_A = (unsigned int)simulator_ecu.ecu_vehicle_speed;
   
   sprintf(reply_buf, "41 0D %.2x\n", vs_A);
   printf("send_vehicle_speed(): Simulator VS Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}


void send_egr_pressure()
{
   /* Has to be calculated. */
   return;
}


int send_throttle_position()
{
   char reply_buf[256];
   unsigned int tp_A;
   int n;
   
   memset(reply_buf, 0, 256);

   tp_A = (unsigned int)simulator_ecu.ecu_throttle_position / 0.392;
   
   sprintf(reply_buf, "41 11 %.2x\n", tp_A);
   /* TODO: log simulator msg. */
   printf("send_throttle_position(): Simulator Throttle Position Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}


int send_oil_temperature()
{
   char reply_buf[256];
   unsigned int ot_A;
   int n;
   
   memset(reply_buf, 0, 256);

   ot_A = (unsigned int)simulator_ecu.ecu_oil_temperature + 40;
   
   sprintf(reply_buf, "41 5C %.2x\n", ot_A);
   /* TODO: log simulator msg. */
   printf("send_oil_temperature(): Simulator OT Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n); 
}


void send_oil_pressure()
{
   /* TODO: use oil pressure switch parameter. */
   return;
}


void send_supported_pid_list_1_32()
{
   return;
}


void send_timing_advance()
{
   return;
}



int send_fuel_tank_level()
{
   char reply_buf[256];
   unsigned int ftl_A;
   int n;
   
   memset(reply_buf, 0, 256);

   ftl_A = (unsigned int)simulator_ecu.ecu_fuel_tank_level / 0.392;
   
   sprintf(reply_buf, "41 2F %.2x\n", ftl_A);
   /* TODO: log simulator msg. */
   printf("send_fuel_tank_level(): Simulator Fuel Tank Level Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}


int send_fuel_flow_rate()
{
   char reply_buf[256];
   unsigned int ffr_temp, ffr_A, ffr_B;
   int n;
   
   memset(reply_buf, 0, 256);

   ffr_temp = (unsigned int)simulator_ecu.ecu_fuel_flow_rate * 20;
   ffr_A = ffr_temp / 256;
   ffr_B = ffr_temp % 256;
   
   sprintf(reply_buf, "41 5E %.2x %.2x\n", ffr_A, ffr_B);
   /* TODO: log simulator msg. */
   printf("send_fuel_flow_rate(): Simulator Fuel Flow Rate Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}

int send_fuel_pressure()
{
   char reply_buf[256];
   unsigned int fp_A;
   int n;
   
   memset(reply_buf, 0, 256);

   fp_A = (unsigned int)simulator_ecu.ecu_fuel_pressure / 3.0;
   
   sprintf(reply_buf, "41 0A %.2x\n", fp_A);
   /* TODO: log simulator msg. */
   printf("send_fuel_pressure(): Simulator Fuel Pressure Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}

int send_accelerator_position()
{
   char reply_buf[256];
   unsigned int ap_A;
   int n;
   
   memset(reply_buf, 0, 256);

   ap_A = (unsigned int)simulator_ecu.ecu_accelerator_position / 0.392;
   
   sprintf(reply_buf, "41 5A %.2x\n", ap_A);
   /* TODO: log simulator msg. */
   printf("send_accelerator_position(): Simulator Accelerator Position Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}

/* OBD Interface Messages. */

void send_battery_voltage()
{
   char reply_buf[256];
   int n;
   
   memset(reply_buf, 0, 256);
   sprintf(reply_buf, "ATRV %.2f\n>\n", simulator_ecu.ecu_battery_voltage);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);

   printf("send_battery_voltage(): Simulator ATRV Msg: %i bytes %s", n, reply_buf);
      
   return;
}

void send_interface_information()
{
   char reply_buf[256];
   int n;
   
   memset(reply_buf, 0, 256);
   /* sprintf(reply_buf, "ATI ELM327\nOK\n>\n"); */

   sprintf(reply_buf, "ATI ELM327\n");
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   printf("send_interface_information(): Simulator ATI Msg: %i bytes %s", n, reply_buf);
   
   return;
}

void send_obd_protocol_name(char *obd_msg)
{
   char reply_buf[256];
   int pnum, n;
   
   memset(simulator_obd.obd_protocol_name, 0, 256);
   memset(reply_buf, 0, 256);
   
   if (strncmp(obd_msg, "ATDP ", 5) == 0)
   {
      sprintf(reply_buf, "ATDP %s\n", simulator_obd.obd_protocol_name);
   }
   else if (sscanf(obd_msg, "ATTP %x", &pnum) == 1)
   {
      if (pnum > 12)
      {
         pnum = 0;
      }   
      simulator_obd.obd_protocol_number = pnum;
      strcpy(simulator_obd.obd_protocol_name, OBD_Protocol_List[pnum]);
      sprintf(reply_buf, "ATTP %s\n", simulator_obd.obd_protocol_name);
   }
   else if (sscanf(obd_msg, "ATSP %x", &pnum) == 1)
   {
      if (pnum > 12)
      {
         pnum = 0;
      }   
      simulator_obd.obd_protocol_number = pnum;
      strcpy(simulator_obd.obd_protocol_name, OBD_Protocol_List[pnum]);
      sprintf(reply_buf, "ATSP %s\n", simulator_obd.obd_protocol_name);
   }
   else
   {
      simulator_obd.obd_protocol_number = 0;
      strncpy(simulator_obd.obd_protocol_name, "Unknown OBD protocol.\n", 22);
      printf("send_obd_protocol_name(): %s", obd_msg);
   }

   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   printf("send_obd_protocol_name() : %i bytes %s", n, reply_buf);
   
   return;
}

void send_vin_msg()
{
   char reply_buf[256];
   int n;
   
   memset(reply_buf, 0, 256);

   sprintf(reply_buf, "%s\n", ecu_vin[0]); /* TODO: switch between CAN and non-CAN formats. */
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   printf("send_vin_msg(): VIN Msg: %i bytes %s", n, reply_buf);
   
   return;
}

void send_ecu_name()
{
   char reply_buf[256];
   int n;
   
   memset(reply_buf, 0, 256);

   sprintf(reply_buf, "%s\n", ecu_name[0]);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   printf("send_ecu_name(): ECU Name: %i bytes %s", n, reply_buf);
   
   return;
}

void send_mil_status()
{
   char reply_buf[256];
   int n, mil_status;
   
   memset(reply_buf, 0, 256);

   mil_status = simulator_ecu.ecu_mil_status * 128 + simulator_ecu.ecu_dtc_count;
   
   sprintf(reply_buf, "41 01 %.2x 01 02 03\n", mil_status); /* Msg = (41 01 81 XX XX XX) if MIL on and 1 DTC. */
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   printf("send_mil_status(): MIL Msg: %i bytes %s", n, reply_buf);
   
   return;
}

void reply_mode_01_msg(char *obd_msg)
{
   /* Send back an ECU parameter message. */
   unsigned int pid, pmode;
   int n;
   
   n = sscanf(obd_msg, "%x %x", &pmode, &pid);
   if (n == 2)
   {
      printf("reply_mode_01_msg(): %d %d\n", pmode, pid);
      switch(pid)
      {
         case 0: send_supported_pid_list_1_32(); break; /* TODO: Supported PIDs. */
         case 1: send_mil_status(); break; /* MIL on/off and DTC count. */
         case 5: send_coolant_temperature(); break; /*  */
         case 10: send_fuel_pressure(); break;
         case 11: send_manifold_pressure(); break; /* Throttle Position. */
         case 12: send_engine_rpm(); break;
         case 13: send_vehicle_speed(); break;
         case 14: send_timing_advance(); break;
         case 15: send_intake_air_temperature(); break;
         case 17: send_throttle_position(); break;
         case 47: send_fuel_tank_level(); break; /* Fuel Tank Level. */
         case 90: send_accelerator_position(); break;
         case 92: send_oil_temperature(); break;
         case 94: send_fuel_flow_rate(); break;
         default: printf("reply_mode_01_msg(): Unknown PID %i\n", pid); break;
      }
   }
   else
   {
      printf("reply_mode_01_msg() <ERROR>: Unknown OBD message.\n");
   }
   return;
}

void reply_mode_03_msg(char *obd_msg)
{
   /* TODO: Send back a DTC message. */
   char reply_buf[256];
   int n;
   
   memset(reply_buf, 0, 256);

   /* TODO: send multiple DTCs. */
   strcpy(reply_buf, "43 01 33 00 00 00 00\n"); /* DTC = P0133. */
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   printf("---------------------------------------------------------\n");
   printf("reply_mode_03_msg(): DTC Msg: %i bytes %s", n, reply_buf);
   printf("---------------------------------------------------------\n");
   
   return;   
   
}

void reply_mode_09_msg(char *obd_msg)
{
   /* Send back ECU and vehicle information message. */
   int n;
   unsigned int pid, pmode;
   
   n = sscanf(obd_msg, "%x %x", &pmode, &pid);
   if (n == 2)
   {
      printf("reply_mode_09_msg(): %d %d\n", pmode, pid);
      switch(pid)
      {
         case 0: break; /* TODO: Supported PIDs. */
         case 1: break; /* TODO: Send ECU name. */
         case 2: send_vin_msg(); break; /* Send VIN number. */  
         case 10: send_ecu_name(); break; /* ECU Name. */ 
      }
   }
   return;
}



int parse_gui_message()
{
   int n;
   int msg_len;
   
   msg_len = strlen(in_buf);
   n = 0;
   
   if ((msg_len > 0) && (in_buf[msg_len - 1] == '\n')) /* All messages must terminate with a newline. */
   {
      /* Parse the message. */
      if (in_buf[0] == 'A') /* ELM327 interface messages all start with 'AT'. */
      {
         if (strncmp(in_buf, "ATRV", 4) == 0)
         {
            send_battery_voltage();
         }
         else if (strncmp(in_buf, "ATI", 3) == 0) /* Get interface name. */
         {
            send_interface_information();
         }
         else if (strncmp(in_buf, "ATDP", 4) == 0) /* Get OBD Protocol. */
         {
            send_obd_protocol_name(in_buf);
         }
         else if (strncmp(in_buf, "ATSP", 4) == 0) /* Set OBD Protocol. */
         {
            send_obd_protocol_name(in_buf);
         }
         else if (strncmp(in_buf, "ATTP", 4) == 0) /* Try OBD Protocol. */
         {
            send_obd_protocol_name(in_buf);
         }
      }
      else
      {                        /* ECU request messages always start with the mode '01'...'0A' */
         if (in_buf[0] == '0') /* This is an OBD request message from the GUI. */
         {
            switch(in_buf[1])
            {
               case '0': break; /* Invalid mode. */
               case '1': reply_mode_01_msg(in_buf); break; /* Mode 01 message, ECU parameter update. */
               case '2': break;
               case '3': reply_mode_03_msg(in_buf); break; /* Mode 03 message, diagnostic trouble codes. */
               case '4': break;
               case '5': break;
               case '6': break;
               case '7': break;
               case '8': break;
               case '9': reply_mode_09_msg(in_buf); break; /* Mode 09 message, ECU information. */
               case 'A': break;
            }
         }
         else /* This is an AT message response for the user interface. */
         {
            /* TODO: Save configuration info from the OBD interface. */
         }
      }
   
   }
   else
   {
      /* TODO: Invalid message, log an error. */
      n = -1;
   }

   return(n);
}

int init_serial_comms(char *interface_name)
{
  int cport_nr=0;        /* /dev/ttyS0 (COM1 on windows) */
  int bdrate=9600;       /* 9600 baud */
  char mode[]={'8','N','1',0};
  
  cport_nr = RS232_GetPortnr(interface_name);
  if (cport_nr == -1)
  {
     printf("ERROR: Can not get com port number.\n");
     exit(-1);
  }

  printf("init_serial_comms(): Serial port number: %i\n",cport_nr);

  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("init_serial_comms() <ERROR>: Can not open comport!\n");
    exit(-1);
  }
  
  return(cport_nr);
}

int send_ecu_query(int serial_port, char *ecu_query)
{
    int out_msg_len = 0;

    out_msg_len = strlen(ecu_query);
    if ((out_msg_len < 1) || (out_msg_len > BUFFER_MAX_LEN))
    {
      printf("send_ecu_query() <ERROR>: Bad message length!\n");
      return(0);
    }

    RS232_cputs(serial_port, ecu_query);

    printf("send_ecu_query(): TXD %i bytes: %s", out_msg_len, ecu_query);

    usleep(100000);  /* sleep for 100 milliseconds */

    return(out_msg_len);
}

int recv_ecu_reply(int serial_port, unsigned char *ecu_query)
{
    int in_msg_len = 0;

    while((in_msg_len = RS232_PollComport(serial_port, ecu_query, BUFFER_MAX_LEN)) > 0)
    {
          int idx;

          ecu_query[in_msg_len] = 0;   /* always put a "null" at the end of a string! */

          for(idx = 0; idx < in_msg_len; idx++)
          {
             if(ecu_query[idx] < 32)  /* replace unreadable control-codes by dots */
             {
                ecu_query[idx] = '.';
             }
          }

          printf("recv_ecu_reply(): RXD %i bytes: %s", in_msg_len, ecu_query);

          usleep(100000);  /* sleep for 100 milliSeconds */
    }

    return(in_msg_len);
}

int main(int argc, char *argv[])
{

   if (argc < 2) 
   {
      fprintf(stderr, "main() <ERROR>: no port provided.\n");
      exit(0);
   }
   
   tick_count = 0;
   set_simulator_ecu_parameters();
   
   /* TODO: make serial port configurable. */
   
   /* TODO: make this configurable for RS232 unit tests.
   serial_port = init_serial_comms("ttyUSB0");
   */
   
   sock = socket(AF_INET, SOCK_DGRAM, 0);

   if (sock < 0) 
      fatal_error("Opening socket");
   
   length = sizeof(server);
   memset(&server, 0, length);

   server.sin_family=AF_INET;
   server.sin_addr.s_addr=INADDR_ANY;
   server.sin_port=htons(atoi(argv[1]));
   
   if (bind(sock, (struct sockaddr *)&server, length) < 0) 
      fatal_error("binding");

   from_len = sizeof(struct sockaddr_in);
   
   memset(in_buf, 0, BUFFER_MAX_LEN);
   
   while (1) 
   {
       n = recvfrom(sock, in_buf, BUFFER_MAX_LEN, 0, (struct sockaddr *)&from_client, &from_len);

       if (n < 0) fatal_error("recvfrom");

       printf("main(): RXD ECU Query: %s", in_buf);

       n = parse_gui_message();

       if (n  < 0) 
          printf("main() <ERROR>:Message parsing failed.\n");

       set_simulator_ecu_parameters();
       
       tick_count += 1;
       if (tick_count == 100)
       {
          tick_count = 0;
       }
       
       /* Now send the query to the ECU interface and get a response. 
       n = send_ecu_query(serial_port, in_buf);
       n = recv_ecu_reply(serial_port, ecu_msg);
       */
       /* TODO: log ECU query and reply. */


       /* TODO: Clear the buffers!!! */
       memset(in_buf, 0, BUFFER_MAX_LEN);
   }

   return 0;
 }

