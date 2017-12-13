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

#include "rs232.h"


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


void reply_mode_01_msg(char *obd_msg)
{
   /* Send back an ECU parameter message. */
   unsigned int pid, pmode;
   int n;
   char response_msg[256];
   
   n = sscanf(obd_msg, "%x %x", &pmode, &pid);
   if (n == 2)
   {
      printf("RPM Msg: %d %d\n", pmode, pid);
      switch(pid)
      {
         case 0: break; /* Request supported PID list. */
         case 5: break; /* ECT  */
         case 11: break;  /* MAP  */
         case 12: break;  /* RPM  */ 
         case 13: break;
         case 15: break;
         case 17: break;
         case 92: break;
         case 94: break;
         case 9: break; /* */
         case 10: break;
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
   /* Send back ECU information message. */
}



int parse_gui_message()
{
   int n;
   int msg_len;
   char temp_buf[256];
   
   msg_len = strlen(in_buf);
   
   if ((msg_len > 0) && (in_buf[msg_len - 1] == '\n')) /* All messages must terminate with a newline. */
   {
      /* Parse the message. */
      if (in_buf[0] == 'A') /* ELM327 interface messages all start with 'AT'. */
      {
         
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

   
   
   
   
   n = sendto(sock, "ACK ECU Request.\n", 18, 0, (struct sockaddr *)&from_client, from_len);
   
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

    printf("TXD %i bytes: %s\n", out_msg_len, ecu_query);

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

          printf("RXD %i bytes: %s\n", in_msg_len, ecu_query);

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
   
   /* TODO: make serial port configurable. */
   
   /* TODO: make this configurable for RS232 unit tests.
   serial_port = init_serial_comms("ttyUSB0");
   */
   
   sock = socket(AF_INET, SOCK_DGRAM, 0);

   if (sock < 0) 
      fatal_error("Opening socket");
   
   length = sizeof(server);
   bzero(&server, length);

   server.sin_family=AF_INET;
   server.sin_addr.s_addr=INADDR_ANY;
   server.sin_port=htons(atoi(argv[1]));
   
   if (bind(sock, (struct sockaddr *)&server, length) < 0) 
      fatal_error("binding");

   from_len = sizeof(struct sockaddr_in);
   
   while (1) 
   {
       bzero(in_buf, BUFFER_MAX_LEN);
       
       n = recvfrom(sock, in_buf, BUFFER_MAX_LEN, 0, (struct sockaddr *)&from_client, &from_len);

       if (n < 0) fatal_error("recvfrom");

       printf("RXD ECU Query: %s\n", in_buf);

       n = parse_gui_message();

       if (n  < 0) 
          printf("Message parsing failed.\n");

       /* Now send the query to the ECU interface and get a response. 
       n = send_ecu_query(serial_port, in_buf);
       n = recv_ecu_reply(serial_port, ecu_msg);
       */
       /* TODO: log ECU query and reply. */

       /* TODO: send ECU reply to GUI. */

       /* TODO: Clear the buffers!!! */
   }

   return 0;
 }

