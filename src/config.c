/* 
   config.c
   
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: Configuration management functions. 


   Date: 18/12/2017
   
*/
#include <stdio.h>
#include <stdlib.h>

#include "obd_monitor.h"
#include "protocols.h"
#include "pid_hash_map.h"
#include "dtc_hash_map.h"

FILE *config_file;

int load_configuration_file(char *config_file)
{
   /* TODO: Define an XML style markup for configuration items. */
   
   return(0);
}

int load_custom_pid_list()
{
   int ii;
   
   /* TODO: Define a custom PID file format using XML style markup. */
   
   for(ii = 0; ii < 10; ii++)
   {
      PID_Parameters *pid = (PID_Parameters *) xmalloc(sizeof(PID_Parameters));
      sprintf(pid->pid_code, "%.4x", ii);
      sprintf(pid->pid_description, "MODE 01: %.4x", ii);
      add_pid(pid); 
   }
   print_pid_map();
   
   return(0);
}

int get_custom_pid(int pid_num)
{
   return(0);
}


int get_config_item()
{
   return(0);
}

int set_config_item()
{
   return(0);
}

int save_configuration_file(char *config_file)
{


   return(0);
}

