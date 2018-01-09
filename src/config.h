/* 
   config.h
   
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: Configuration management functions. 


   Date: 18/12/2017
   
*/

#ifndef OBD_CONFIG_INCLUDED
#define OBD_CONFIG_INCLUDED


struct _ConfigOptions {
   char log_file_name[MAX_PATH_LEN];
   char custom_pid_file_name[MAX_PATH_LEN];
   char config_file_name[MAX_PATH_LEN];
   int ecu_auto_connect; /* 0,1 or delay in seconds. */
};

typedef struct _ConfigOptions ConfigOptions;

int load_configuration_file(char *config_file);
int load_custom_pid_list();
int get_custom_pid(int pid_num);


#endif
