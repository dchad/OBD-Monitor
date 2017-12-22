/* gui_dialogs.h


   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A GUI for communication with vehicle engine control units via 
                an OBD-II interface to obtain and display engine status 
                and fault codes. 


   Date: 30/11/2017
   
*/

#ifndef OBD_DIALOGS_INCLUDED
#define OBD_DIALOGS_INCLUDED

void init_dialogs();
void open_custom_pid_dialog();
void send_custom_pid_query(GtkWidget *widget, gpointer window);

/* Dialog Functions. */
void show_info_dialog(GtkWidget *widget, gpointer window);
void show_error_dialog(GtkWidget *widget, gpointer window);
void show_status_dialog(GtkWidget *widget, gpointer window);
void show_warning_dialog(GtkWidget *widget, gpointer window);
void show_question_dialog(GtkWidget *widget, gpointer window);

/* Message Setting Functions. */
void set_info_msg(char *msg);
void set_error_msg(char *msg);
void set_status_msg(char *msg);
void set_warning_msg(char *msg);
void set_question_msg(char *msg);

void get_status_msg(char *msg);

#endif


