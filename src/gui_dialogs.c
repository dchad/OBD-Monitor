/* gui_dialogs.c 


   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A GUI for communication with vehicle engine control units via 
                an OBD-II interface to obtain and display engine status 
                and fault codes. 


   Date: 30/11/2017
   
*/

#include <gtk/gtk.h>
#include <math.h>
#include "obd_monitor.h"
#include "protocols.h"


/* Status, error and warning message buffers. */
char current_info_msg[256];
char current_error_msg[256];
char current_status_msg[256];
char current_warning_msg[256];
char current_question_msg[256];


/* Buffers for Engine Control Unit messages. */
char ECU_PID_Request[256];
char ECU_PID_Reply[256];

/* Buffers for OBD Interface messages. */
char ODB_Request[256];
char OBD_Reply[256];


void set_info_msg(char *msg)
{
   strncpy(current_info_msg, msg, 256);
   return;
}

void set_error_msg(char *msg)
{
   memset(current_error_msg, 0, 256);
   strncpy(current_error_msg, msg, 256);
   return;
}

void set_status_msg(char *msg)
{
   memset(current_status_msg, 0, 256);
   strncpy(current_status_msg, msg, 256);
   return;
}

void set_warning_msg(char *msg)
{
   memset(current_warning_msg, 0, 256);
   strncpy(current_warning_msg, msg, 256);
   return;
}

void set_question_msg(char *msg)
{

   memset(current_question_msg, 0, 256);
   strncpy(current_question_msg, msg, 256);
   return;
}

void init_dialogs()
{

   return;
}


void send_custom_pid_query(GtkWidget *widget, gpointer window) 
{
   char buffer[256];
   /* TODO: this is for custom PID dialog box send button. */
   memset(buffer, 0, 256);
   memset(ECU_PID_Request, 0, 16);
   sprintf(ECU_PID_Request,"0100");
   send_ecu_msg(ECU_PID_Request);
   usleep(OBD_WAIT_TIMEOUT);
   recv_ecu_msg(buffer);

   return;
}

void show_info_dialog(GtkWidget *widget, gpointer window) 
{
    
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "%s",
            current_info_msg);
  gtk_window_set_title(GTK_WINDOW(dialog), "Information");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  return;
}

void show_error_dialog(GtkWidget *widget, gpointer window) 
{
    
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "%s", 
            current_error_msg);
  gtk_window_set_title(GTK_WINDOW(dialog), "Error");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  return;
}


void show_question_dialog(GtkWidget *widget, gpointer window) 
{
    
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "Are you sure to quit?");
  gtk_window_set_title(GTK_WINDOW(dialog), "Question");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  return;
}

void show_warning_dialog(GtkWidget *widget, gpointer window) 
{
    
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "%s",
            current_warning_msg);
  gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  return;
}


