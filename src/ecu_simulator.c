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
unsigned int tick_count;

int sock, length, n, serial_port;
socklen_t from_len;
struct sockaddr_in server;
struct sockaddr_in from_client;
char in_buf[BUFFER_MAX_LEN];
unsigned char ecu_msg[BUFFER_MAX_LEN];
   
   
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
   simulator_ecu.ecu_battery_voltage = 12.6;
   simulator_ecu.ecu_throttle_position = (double)tick_count;
   simulator_ecu.ecu_oil_temperature = 2.0 * (double)tick_count;
   simulator_ecu.ecu_accelerator_position = (double)tick_count;
   simulator_ecu.ecu_fuel_pressure = 3.0 * (double)tick_count;
   simulator_ecu.ecu_fuel_flow_rate = 2.0 * (double)tick_count;
    
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
   printf("Simulator RPM Msg: %s", reply_buf);
   
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
   printf("Simulator ECT Msg: %s", reply_buf);
   
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
   
   printf("Simulator MAP Msg: %i bytes %s", n, reply_buf);
      
   return;
}


void send_intake_air_temperature()
{

   
   return;
}


int send_vehicle_speed()
{
   char reply_buf[256];
   unsigned int vs_A;
   int n;
   
   memset(reply_buf, 0, 256);

   vs_A = (unsigned int)simulator_ecu.ecu_vehicle_speed;
   
   sprintf(reply_buf, "41 0D %.2x\n", vs_A);
   printf("Simulator VS Msg: %s", reply_buf);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   return(n);
}


void send_egr_pressure()
{

}


void send_throttle_position()
{

}


void send_oil_temperature()
{
 
   return;
}


void send_oil_pressure()
{

}


void send_supported_pid_list_1_32()
{

}


void send_timing_advance()
{

}



void send_fuel_tank_level()
{

}


void send_fuel_flow_rate()
{

}

void send_fuel_pressure()
{

}

void send_accelerator_position()
{

}

/* OBD Interface Messages. */

void send_battery_voltage()
{
   char reply_buf[256];
   int n;
   
   memset(reply_buf, 0, 256);
   sprintf(reply_buf, "ATRV %.2f\n>\n", simulator_ecu.ecu_battery_voltage);
   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);

   printf("Simulator ATRV Msg: %i bytes %s", n, reply_buf);
      
   return;
}

void send_interface_information()
{
   char reply_buf[256];
   int n;
   
   memset(reply_buf, 0, 256);
   sprintf(reply_buf, "ATI ELM327\nOK\n>\n");

   
   n = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&from_client, from_len);
   
   printf("Simulator ATI Msg: %i bytes %s", n, reply_buf);
   
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
      printf("MODE 01 Msg: %d %d\n", pmode, pid);
      switch(pid)
      {
         case 0: send_supported_pid_list_1_32(); break; /* TODO: Supported PIDs. */
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
      }
   }
   else
   {
   
   }
   return;
}

void reply_mode_03_msg(char *obd_msg)
{
   /* Send back a DTC message. */
   
}

void reply_mode_09_msg(char *obd_msg)
{
   /* Send back ECU and vehicle information message. */
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
         if (strncmp(in_buf, "ATI", 3) == 0)
         {
            send_interface_information();
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
               case '9': reply_mode_03_msg(in_buf); break; /* Mode 09 message, ECU information. */
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

  printf("Serial port number: %i\n",cport_nr);

  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("ERROR: Can not open comport!\n");
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
      printf("ERROR: Bad message length!\n");
      return(0);
    }

    RS232_cputs(serial_port, ecu_query);

    printf("TXD %i bytes: %s", out_msg_len, ecu_query);

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

          printf("RXD %i bytes: %s", in_msg_len, ecu_query);

          usleep(100000);  /* sleep for 100 milliSeconds */
    }

    return(in_msg_len);
}

int main(int argc, char *argv[])
{

   if (argc < 2) 
   {
      fprintf(stderr, "ERROR: no port provided.\n");
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

       printf("RXD ECU Query: %s", in_buf);

       n = parse_gui_message();

       if (n  < 0) 
          printf("Message parsing failed.\n");

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

