/* 
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
#include "gui_dialogs.h"
#include "gui_gauges.h"
#include "gui_gauges_aux.h"

  
GtkWidget *window;
  
/* Current time string. */
char time_buffer[256];
char status_bar_msg[256];
char obd_protocol[256];

/* Communications log display area. */
GtkTextBuffer *comms_text_buffer;
GtkTextBuffer *pid_text_buffer;
GtkTextBuffer *dtc_text_buffer;
GtkTextIter text_iter;

/* TODO: Change to PID struct list. */
char *aux_gauge_pid_list[] = {
"Auxilliary One",
"Auxilliary Two",
"Auxilliary Three",
"Auxilliary Four",
"Auxilliary Five",
"Auxilliary Six",
"Auxilliary Seven",
"Auxilliary Eight",
"Auxilliary Nine",
"Auxilliary Plus" };

/* ----------------------- */
/* GUI callback functions. */
/* ----------------------- */

void update_comms_log_view(char *msg)
{
   char temp_buf[256];
   int len = strlen(msg);
   
   memset(temp_buf, 0, 256);
   
   if (msg[len-1] != '\n')
      sprintf(temp_buf, "%s\n", msg);
   else
      strcpy(temp_buf, msg);
      
   gtk_text_buffer_get_iter_at_offset(comms_text_buffer, &text_iter, -1);
   gtk_text_buffer_insert(comms_text_buffer, &text_iter, temp_buf, -1);
   
   return;
}

void set_status_bar_msg(char *msg)
{
   memset(status_bar_msg, 0, 256);
   if (strlen(msg) > 0)
   {
      strncpy(status_bar_msg, msg, strlen(msg));
      g_strchomp(status_bar_msg); /* Remove trailing whitespace. */
      update_comms_log_view(status_bar_msg);
   }
   else
   {
      strcpy(status_bar_msg, "Invalid Message.");
   }
   
   return;
}

void get_status_bar_msg(char *msg)
{
   memset(msg, 0, 256);
   strncpy(msg, status_bar_msg, strlen(status_bar_msg));
   return;
}


void protocol_combo_selected(GtkComboBoxText *widget, gpointer window) 
{
  gchar *text = gtk_combo_box_text_get_active_text(widget);
  int selected;
  
  memset(obd_protocol, 0, 256);
  
  if (text != NULL)
  {
     selected = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
     printf("protocol_combo_selected() : Protocol Selected: %i %s\n", selected, text);
     sprintf(obd_protocol, "ATSP %.2x\r", selected);
     /* send_ecu_msg(obd_protocol); */
  }
  g_free(text);
  
  return;
}

GdkPixbuf *create_pixbuf(const gchar * filename) 
{
    
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   
   if (!pixbuf) 
   {
      printf("%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

void print_msg(GtkWidget *widget, gpointer window) 
{
   printf("Button clicked\n");

   return;
}

gint send_obd_message_60sec_callback (gpointer data)
{
   send_ecu_msg("ATRV\r");  /* Battery Voltage */
   send_ecu_msg("01 01\r"); /* Get MIL status and DTC count. */
   send_ecu_msg("03\r");    /* Get DTC codes. */
   send_ecu_msg("01 05\r"); /* Coolant Temperature */
   send_ecu_msg("01 2F\r"); /* Fuel Tank Level */
   send_ecu_msg("01 0F\r"); /* Intake Air Temperature */
   send_ecu_msg("01 5C\r"); /* Oil Temperature */
   send_ecu_msg("01 0A\r"); /* Fuel Pressure */
   send_ecu_msg("01 0B\r"); /* MAP Pressure */
   send_ecu_msg("01 5E\r"); /* Fuel Flow Rate */
   
   return(TRUE);
}

gint send_obd_message_30sec_callback (gpointer data)
{
   send_ecu_msg("01 05\r"); /* Coolant Temperature */
   send_ecu_msg("01 2F\r"); /* Fuel Tank Level */
   send_ecu_msg("01 0F\r"); /* Intake Air Temperature */
   send_ecu_msg("01 5C\r"); /* Oil Temperature */
    
   return(TRUE);
}

gint send_obd_message_10sec_callback (gpointer data)
{
   send_ecu_msg("01 05\r"); /* Coolant Temperature */
   send_ecu_msg("01 2F\r"); /* Fuel Tank Level */
   send_ecu_msg("01 0F\r"); /* Intake Air Temperature */
   send_ecu_msg("01 5C\r"); /* Oil Temperature */
   
   return(TRUE);
}

gint send_obd_message_1sec_callback (gpointer data)
{

   send_ecu_msg("01 0C\r"); /* Engine RPM */
   send_ecu_msg("01 0D\r"); /* Vehicle Speed */
   
   return(TRUE);
}

gint recv_obd_message_callback (gpointer data)
{
   char msg_buf[256];
   int n, msg_num;
   
   memset(msg_buf, 0, 256);
   
   n = recv_ecu_msg(msg_buf);
   if (n > 0)
   {
      msg_num = parse_obd_msg(msg_buf);
      if (msg_num < 0)
      {
         /* TODO: Log an error message. */

      }
      else
      {
         /* TODO: set status bar and log view messages and remove dependencies from protocols.c
            Change message parser to parse_obd_msg(buffer, status_msg);
         */
      }
      gtk_widget_queue_draw(window);
   }
   
   return(TRUE);
}

void ecu_connect_callback(GtkWidget *widget, gpointer window) 
{
   char rcv_msg_buf[256];
   int msg_num;

   if (get_ecu_connected() == 1)
   {
      return;
   }
   
   if (strlen(obd_protocol) < 5)
   {
      strcpy(obd_protocol, "ATTP 0\r");
   }
   
   if (server_connect() > 0)
   {
      init_obd_comms(obd_protocol);
      
      send_ecu_msg("ATDP\r");  /* Get OBD protocol name from interface. */
      send_ecu_msg("ATRV\r");  /* Get battery voltage from interface. */
      send_ecu_msg("09 02\r"); /* Get vehicle VIN number. */
      send_ecu_msg("09 0A\r"); /* Get ECU name. */
      send_ecu_msg("01 01\r"); /* Get DTC Count and MIL status. */
      send_ecu_msg("01 00\r"); /* Get supported PIDs 1 - 32 for MODE 1. */
      send_ecu_msg("09 00\r"); /* Get supported PIDs 1 - 32 for MODE 9. */
      send_ecu_msg("03\r");      
      
      g_timeout_add (60000, send_obd_message_60sec_callback, (gpointer)window);
      g_timeout_add (1000, send_obd_message_1sec_callback, (gpointer)window);
      g_timeout_add (100, recv_obd_message_callback, (gpointer)window);  
 
   }
   else
   {
      /* TODO: popup error dialog. */
      printf("ecu_connect_callback(): <ERROR> Could not connect to server.\n");
   }

   return;
}


gboolean image_press_callback(GtkWidget *event_box, GdkEventButton *event, gpointer data)
{
   g_print ("Event box clicked at coordinates %f,%f\n", event->x, event->y);

   /* Returning TRUE means we handled the event, so the signal
      emission should be stopped (don’t call any further callbacks
      that may be connected). Return FALSE to continue invoking callbacks. */
  
   return TRUE;
}

GtkWidget* create_image(char *image_file, GtkWidget *cbox)
{
  GtkWidget *image;
  GtkWidget *event_box;
  GtkWidget *info_label, *notice_label;


  image = gtk_image_new_from_file (image_file);
  info_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (info_label), "<b>THIS FUNCTION NOT IMPLEMENTED YET SAD FACE!</b>");
  notice_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (notice_label), "<b>INFORMATIONAL SAD FACE</b>");

  event_box = gtk_event_box_new ();

  
  gtk_container_add (GTK_CONTAINER (event_box), image);
  gtk_box_pack_start (GTK_BOX (cbox), notice_label, FALSE, FALSE, 50);
  gtk_box_pack_start (GTK_BOX (cbox), event_box, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(cbox), info_label, FALSE, FALSE, 10);

  g_signal_connect (G_OBJECT (event_box),
                    "button_press_event",
                    G_CALLBACK (image_press_callback),
                    image);

  return image;
}

GtkWidget *create_tab_panel(GtkWidget *window, GtkWidget *instruments_vbox, GtkWidget *communications_vbox, 
                            GtkWidget *pid_vbox, GtkWidget *dtc_vbox, GtkWidget *performance_vbox,
                            GtkWidget *auxilliary_vbox)
{
   GtkWidget *notebook;
   GtkWidget *tab_instruments_label;
   GtkWidget *tab_auxilliary_label;
   GtkWidget *tab_communications_label;
   GtkWidget *tab_PID_label;
   GtkWidget *tab_DTC_label;
   GtkWidget *tab_performance_label;
   GtkWidget *image_sad;
   GtkWidget *image_happy;


   image_sad = create_image("../images/face-sad.png", pid_vbox);
   image_sad = create_image("../images/face-sad.png", dtc_vbox);
   image_sad = create_image("../images/face-sad.png", performance_vbox);
   /* image_sad = create_image("../images/face-sad.png", auxilliary_vbox); */
    
   
   notebook = gtk_notebook_new ();
   gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
   
   tab_instruments_label = gtk_label_new ("Instruments");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), instruments_vbox, tab_instruments_label);
	
	tab_auxilliary_label = gtk_label_new ("Auxilliary Gauges");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), auxilliary_vbox, tab_auxilliary_label);
	
   tab_DTC_label = gtk_label_new ("DTC List");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), dtc_vbox, tab_DTC_label);
	
   tab_PID_label = gtk_label_new ("PID List");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), pid_vbox, tab_PID_label);
	
   tab_performance_label = gtk_label_new ("Performance");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), performance_vbox, tab_performance_label);

   tab_communications_label = gtk_label_new ("Communications Log");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), communications_vbox, tab_communications_label);
	
   return(notebook);
}


void init_gui(GtkWidget *window)
{
   /* TODO: move all the gui initialisation from main to here. */
   
   return;
}


int main(int argc, char *argv[]) 
{
 
   GtkWidget *tab_panel;

   GtkWidget *dtc_button;
   GtkWidget *egr_button;
   GtkWidget *iat_button;
   GtkWidget *map_button;
   GtkWidget *pid_button;
   GtkWidget *ecu_request_button;
   GtkWidget *ecu_connect_button;

   GtkWidget *instruments_vbox;
   GtkWidget *auxilliary_vbox;
   GtkWidget *communications_vbox;
   GtkWidget *pid_vbox;
   GtkWidget *dtc_vbox;
   GtkWidget *performance_vbox;
   GtkWidget *vbox_controls;
   GtkWidget *hbox_top;
   GtkWidget *hbox_center;
   GtkWidget *hbox_bottom;
   GtkWidget *vbox_left;
   GtkWidget *vbox_center;
   GtkWidget *vbox_right;
   GtkWidget *vbox;
 
   GtkWidget *hbox_aux_top;
   GtkWidget *hbox_aux_mid;
   GtkWidget *hbox_aux_bot;
   GtkWidget *hbox_aux_left;
   GtkWidget *hbox_aux_right;

   GtkWidget *menubar;
   GtkWidget *fileMenu;
   GtkWidget *editMenu;
   GtkWidget *helpMenu;

   GtkWidget *editMenuItem;
   GtkWidget *optionsMenuItem;
   GtkWidget *layoutMenuItem;

   GtkWidget *helpMenuItem;
   GtkWidget *aboutMenuItem;
   GtkWidget *manualMenuItem;

   GtkWidget *fileMenuItem;
   GtkWidget *saveMenuItem;
   GtkWidget *quitMenuItem;

   GtkWidget *protocol_combo_box;

   GtkWidget *combo_label;
   GtkWidget *status_label;
   GtkWidget *log_label;
   GtkWidget *instruments_label;
   GtkWidget *text_view_label;

   GtkWidget *ecu_rpm_dial;
   GtkWidget *ecu_speed_dial;
   GtkWidget *ecu_ect_dial;
   GtkWidget *ecu_iat_dial;
   GtkWidget *ecu_map_dial;
   GtkWidget *ecu_egr_dial;
   GtkWidget *ecu_oil_temp_dial;
   GtkWidget *ecu_oil_pressure_dial;
   GtkWidget *ecu_fuel_flow_dial;
   GtkWidget *ecu_fuel_pressure_dial;
   GtkWidget *ecu_fuel_tank_level_dial;
   GtkWidget *battery_voltage_dial;
   GtkWidget *notification_dial;
   
   GtkWidget *aux_dial_1;
   GtkWidget *aux_dial_2;
   GtkWidget *aux_dial_3;
   GtkWidget *aux_dial_4;
   GtkWidget *aux_dial_5;
   GtkWidget *aux_dial_6;
   GtkWidget *aux_dial_7;
   GtkWidget *aux_dial_8;
   GtkWidget *aux_dial_9;

   GtkWidget *aux_dial_grid;
   
   GtkWidget *dtc_dial;
   GtkWidget *pid_dial;

   GtkWidget *comms_text_view;
   GtkWidget *comms_text_frame;
   GtkWidget *comms_scrolled_window;
   
   GtkWidget *status_frame;
   
   
   char rcv_msg_buf[256];
   
   open_log_file("./", "obd_gui_log.txt");

   gtk_init(&argc, &argv);

   printf("GTK+ version: %d.%d.%d\n", gtk_major_version, gtk_minor_version, gtk_micro_version);
   printf("Glib version: %d.%d.%d\n", glib_major_version, glib_minor_version, glib_micro_version);    


   /* Set up the main window. */
   window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "OBD-II Monitor");
   gtk_window_set_default_size(GTK_WINDOW(window), 1000, 600);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 5);

   /* Set up the main menu bar. */
   menubar = gtk_menu_bar_new();
   fileMenu = gtk_menu_new();
   fileMenuItem = gtk_menu_item_new_with_label("File");
   saveMenuItem = gtk_menu_item_new_with_label("Save");
   quitMenuItem = gtk_menu_item_new_with_label("Quit");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMenuItem), fileMenu);
   gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveMenuItem);
   gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMenuItem); 
   gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMenuItem);
   g_signal_connect(G_OBJECT(quitMenuItem), "activate", G_CALLBACK(gtk_main_quit), NULL);

   editMenu = gtk_menu_new();
   editMenuItem = gtk_menu_item_new_with_label("Edit");
   optionsMenuItem = gtk_menu_item_new_with_label("Options");
   g_signal_connect(G_OBJECT(optionsMenuItem), "activate", G_CALLBACK(print_msg), NULL);
   layoutMenuItem = gtk_menu_item_new_with_label("Layout");
   g_signal_connect(G_OBJECT(layoutMenuItem), "activate", G_CALLBACK(print_msg), NULL);
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(editMenuItem), editMenu);
   gtk_menu_shell_append(GTK_MENU_SHELL(editMenu), optionsMenuItem);
   gtk_menu_shell_append(GTK_MENU_SHELL(editMenu), layoutMenuItem); 
   gtk_menu_shell_append(GTK_MENU_SHELL(menubar), editMenuItem);

   helpMenu = gtk_menu_new();
   helpMenuItem = gtk_menu_item_new_with_label("Help");
   aboutMenuItem = gtk_menu_item_new_with_label("About");
   g_signal_connect(G_OBJECT(aboutMenuItem), "activate", G_CALLBACK(print_msg), NULL);
   manualMenuItem = gtk_menu_item_new_with_label("Manual");
   g_signal_connect(G_OBJECT(manualMenuItem), "activate", G_CALLBACK(print_msg), NULL);
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(helpMenuItem), helpMenu);
   gtk_menu_shell_append(GTK_MENU_SHELL(helpMenu), aboutMenuItem);
   gtk_menu_shell_append(GTK_MENU_SHELL(helpMenu), manualMenuItem); 
   gtk_menu_shell_append(GTK_MENU_SHELL(menubar), helpMenuItem);

   /* Create the tab panels. */
   instruments_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   communications_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   pid_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   dtc_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   performance_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   auxilliary_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
   tab_panel = create_tab_panel(window, instruments_vbox, communications_vbox, pid_vbox, dtc_vbox, performance_vbox, auxilliary_vbox);
   

   /* Set up the buttons. */
   dtc_button = gtk_button_new_with_mnemonic("_DTC Lookup");
   g_signal_connect(dtc_button, "clicked", G_CALLBACK(show_info_dialog), NULL); 
   gtk_widget_set_tooltip_text(dtc_button, "Button widget");

   iat_button = gtk_button_new_with_mnemonic("_IAT");
   g_signal_connect(iat_button, "clicked", G_CALLBACK(show_error_dialog), NULL); 
   gtk_widget_set_tooltip_text(iat_button, "Button widget");

   map_button = gtk_button_new_with_mnemonic("_MAP");
   g_signal_connect(map_button, "clicked", G_CALLBACK(show_warning_dialog), NULL); 
   gtk_widget_set_tooltip_text(map_button, "Button widget");

   egr_button = gtk_button_new_with_mnemonic("_TEG");
   g_signal_connect(egr_button, "clicked", G_CALLBACK(show_question_dialog), NULL); 
   gtk_widget_set_tooltip_text(egr_button, "Button widget");

   pid_button = gtk_button_new_with_mnemonic("_PID Lookup");
   g_signal_connect(pid_button, "clicked", G_CALLBACK(show_question_dialog), NULL); 
   gtk_widget_set_tooltip_text(pid_button, "Button widget");

   ecu_request_button = gtk_button_new_with_mnemonic("ECU _Request");
   g_signal_connect(ecu_request_button, "clicked", G_CALLBACK(send_custom_pid_query), NULL); 
   gtk_widget_set_tooltip_text(ecu_request_button, "Button widget");

   ecu_connect_button = gtk_button_new_with_mnemonic("_ECU Connect");
   g_signal_connect(ecu_connect_button, "clicked", G_CALLBACK(ecu_connect_callback), NULL); 
   gtk_widget_set_tooltip_text(ecu_connect_button, "Button widget");


   /* Set up ECU dials. */
   ecu_rpm_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_rpm_dial, 200, 150);
   g_signal_connect(ecu_rpm_dial, "draw", G_CALLBACK(draw_rpm_dial), NULL);
   
   ecu_speed_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_speed_dial, 200, 150);
   g_signal_connect(ecu_speed_dial, "draw", G_CALLBACK(draw_speed_dial), NULL);
   
   ecu_ect_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_ect_dial, 200, 150);
   g_signal_connect(ecu_ect_dial, "draw", G_CALLBACK(draw_ect_dial), NULL);
   
   ecu_iat_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_iat_dial, 200, 150);
   g_signal_connect(ecu_iat_dial, "draw", G_CALLBACK(draw_iat_dial), NULL);
   
   ecu_map_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_map_dial, 200, 150);
   g_signal_connect(ecu_map_dial, "draw", G_CALLBACK(draw_map_dial), NULL);
   
   /* ecu_egr_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_egr_dial, 200, 150);
   g_signal_connect(ecu_egr_dial, "draw", G_CALLBACK(draw_egr_dial), NULL); */
   
   ecu_fuel_pressure_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_fuel_pressure_dial, 200, 150);
   g_signal_connect(ecu_fuel_pressure_dial, "draw", G_CALLBACK(draw_fuel_pressure_dial), NULL);
   
   ecu_oil_temp_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_oil_temp_dial, 200, 150);
   g_signal_connect(ecu_oil_temp_dial, "draw", G_CALLBACK(draw_oil_temp_dial), NULL);
   
   /* ecu_oil_pressure_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_oil_pressure_dial, 200, 150);
   g_signal_connect(ecu_oil_pressure_dial, "draw", G_CALLBACK(draw_oil_pressure_dial), NULL); */
   
   ecu_fuel_tank_level_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_fuel_tank_level_dial, 200, 150);
   g_signal_connect(ecu_fuel_tank_level_dial, "draw", G_CALLBACK(draw_fuel_tank_level_dial), NULL);
   
   ecu_fuel_flow_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_fuel_flow_dial, 200, 150);
   g_signal_connect(ecu_fuel_flow_dial, "draw", G_CALLBACK(draw_fuel_flow_dial), NULL);

   dtc_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (dtc_dial, 300, 230);
   g_signal_connect(dtc_dial, "draw", G_CALLBACK(draw_dtc_dial), NULL);
   
   pid_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (pid_dial, 300, 230);
   g_signal_connect(pid_dial, "draw", G_CALLBACK(draw_pid_dial), NULL);
   
   battery_voltage_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (battery_voltage_dial, 300, 40);
   g_signal_connect(battery_voltage_dial, "draw", G_CALLBACK(draw_battery_voltage_dial), NULL); 
   
   notification_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (notification_dial, 990, 40);
   g_signal_connect(notification_dial, "draw", G_CALLBACK(draw_notification_dial), NULL);     

   /* Gauges for custom PIDs. */
   aux_dial_1 = gtk_drawing_area_new();
   gtk_widget_set_size_request (aux_dial_1, 200, 150);
   g_signal_connect(aux_dial_1, "draw", G_CALLBACK(draw_aux_dial), (gpointer) aux_gauge_pid_list[0]);
 
   aux_dial_2 = gtk_drawing_area_new();
   gtk_widget_set_size_request (aux_dial_2, 200, 150);
   g_signal_connect(aux_dial_2, "draw", G_CALLBACK(draw_aux_dial), (gpointer) aux_gauge_pid_list[1]);
   
   aux_dial_3 = gtk_drawing_area_new();
   gtk_widget_set_size_request (aux_dial_3, 200, 150);
   g_signal_connect(aux_dial_3, "draw", G_CALLBACK(draw_aux_dial), (gpointer) aux_gauge_pid_list[2]);
   
   aux_dial_4 = gtk_drawing_area_new();
   gtk_widget_set_size_request (aux_dial_4, 200, 150);
   g_signal_connect(aux_dial_4, "draw", G_CALLBACK(draw_aux_dial), (gpointer) aux_gauge_pid_list[3]);
   
   aux_dial_5 = gtk_drawing_area_new();
   gtk_widget_set_size_request (aux_dial_5, 200, 150);
   g_signal_connect(aux_dial_5, "draw", G_CALLBACK(draw_aux_dial), (gpointer) aux_gauge_pid_list[4]);
   
   aux_dial_6 = gtk_drawing_area_new();
   gtk_widget_set_size_request (aux_dial_6, 200, 150);
   g_signal_connect(aux_dial_6, "draw", G_CALLBACK(draw_aux_dial), (gpointer) aux_gauge_pid_list[5]);
   
   aux_dial_7 = gtk_drawing_area_new();
   gtk_widget_set_size_request (aux_dial_7, 200, 150);
   g_signal_connect(aux_dial_7, "draw", G_CALLBACK(draw_aux_dial), (gpointer) aux_gauge_pid_list[6]);
   
   aux_dial_8 = gtk_drawing_area_new();
   gtk_widget_set_size_request (aux_dial_8, 200, 150);
   g_signal_connect(aux_dial_8, "draw", G_CALLBACK(draw_aux_dial), (gpointer) aux_gauge_pid_list[7]);  
   
   aux_dial_9 = gtk_drawing_area_new();
   gtk_widget_set_size_request (aux_dial_9, 200, 150);
   g_signal_connect(aux_dial_9, "draw", G_CALLBACK(draw_aux_dial), (gpointer) aux_gauge_pid_list[8]);
      
   aux_dial_grid = gtk_grid_new ();
   gtk_grid_set_row_spacing (GTK_GRID(aux_dial_grid), 10);
   gtk_grid_set_column_spacing (GTK_GRID(aux_dial_grid), 10);
   gtk_grid_attach(GTK_GRID(aux_dial_grid), aux_dial_1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(aux_dial_grid), aux_dial_2, 1, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(aux_dial_grid), aux_dial_3, 2, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(aux_dial_grid), aux_dial_4, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(aux_dial_grid), aux_dial_5, 1, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(aux_dial_grid), aux_dial_6, 2, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(aux_dial_grid), aux_dial_7, 0, 2, 1, 1);
   gtk_grid_attach(GTK_GRID(aux_dial_grid), aux_dial_8, 1, 2, 1, 1);
   gtk_grid_attach(GTK_GRID(aux_dial_grid), aux_dial_9, 2, 2, 1, 1);


   /* Set up other widgets. */

   protocol_combo_box = gtk_combo_box_text_new();
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "0", "0 - Automatic OBD-II Protocol Search");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "1", "1 - SAE J1850 PWM (41.6 kbaud)(Ford)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "2", "2 - SAE J1850 VPW (10.4 kbaud)(GM, Isuzu)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "3", "3 - IS0 9141-2 (5 baud init, 10.4 kbaud)(Chrysler)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "4", "4 - ISO 14230-4 KWP2000 (5-baud init.)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "5", "5 - IS0 14230-4 KWP2000 (Fast init.)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "6", "6 - IS0 15765-4 CAN (11 bit ID, 500 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "7", "7 - IS0 15765-4 CAN (29 bit ID, 500 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "8", "8 - IS0 15765-4 CAN (11 bit ID, 250 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "9", "9 - IS0 15765-4 CAN (29 bit ID, 250 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "A", "A - SAE J1939 CAN (29 bit ID, 250 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "B", "B - USER1 CAN (11 bit ID, 125 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), "C", "C - USER2 CAN (11 bit ID, 50 kbaud)");
   g_signal_connect(protocol_combo_box, "changed", G_CALLBACK(protocol_combo_selected), NULL);

   /* Communications Log Text View Widget and Text Buffer. */
   comms_text_view = gtk_text_view_new ();
   comms_text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (comms_text_view));
   gtk_widget_set_size_request (comms_text_view, 880, 600);
   gtk_text_buffer_get_iter_at_offset(comms_text_buffer, &text_iter, 0);
   gtk_text_buffer_insert(comms_text_buffer, &text_iter, "OBD Monitor Initialising...\n", -1);
   comms_text_frame = gtk_frame_new("Communications Log");
   comms_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (comms_scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
   
   gtk_container_add (GTK_CONTAINER (comms_scrolled_window), comms_text_view);
   gtk_container_add (GTK_CONTAINER (comms_text_frame), comms_scrolled_window);
   gtk_container_set_border_width(GTK_CONTAINER(comms_text_frame), 5);

   status_frame = gtk_frame_new("Nofifications");
  
  
   /* Set up labels. */   
   combo_label = gtk_label_new (NULL);
   gtk_label_set_markup (GTK_LABEL (combo_label), "<b>OBD-II Protocol: </b>");
   instruments_label = gtk_label_new (NULL);
   gtk_label_set_markup (GTK_LABEL (instruments_label), "<b>Instruments: </b>");
   text_view_label = gtk_label_new(NULL);
   gtk_label_set_markup (GTK_LABEL (text_view_label), "<b>Communications Log</b>");

   
   /* gtk_container_add(GTK_CONTAINER(window), vbox); */

   /* Set up the instruments tab container layout. */
   vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   vbox_left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   vbox_center = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   vbox_right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   hbox_top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
   hbox_center = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
   hbox_bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
   vbox_controls = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   
   
   gtk_container_add(GTK_CONTAINER(window), vbox); /* Contains menu bar and tab panel. */
   
   gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(vbox), tab_panel, FALSE, FALSE, 0);
  
   /* Now pack the instruments tab panel. */
   gtk_box_pack_start(GTK_BOX(instruments_vbox), hbox_top, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(instruments_vbox), hbox_center, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(instruments_vbox), hbox_bottom, FALSE, FALSE, 0);
   
   gtk_box_pack_start(GTK_BOX(hbox_top), combo_label, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_top), protocol_combo_box, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_top), ecu_connect_button, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_top), ecu_request_button, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_top), battery_voltage_dial, TRUE, TRUE, 0);
   
   gtk_container_add (GTK_CONTAINER (status_frame), notification_dial);
   gtk_box_pack_start(GTK_BOX(hbox_bottom), status_frame, TRUE, TRUE, 0);
   
   gtk_box_pack_start(GTK_BOX(hbox_center), vbox_left, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_center), vbox_center, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_center), vbox_right, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_center), vbox_controls, TRUE, TRUE, 0);
   
   gtk_box_pack_start(GTK_BOX(vbox_controls), dtc_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_controls), pid_dial, TRUE, TRUE, 0); 
   
   gtk_box_pack_start(GTK_BOX(vbox_left), ecu_rpm_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_left), ecu_ect_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_left), ecu_iat_dial, TRUE, TRUE, 0);
   
   gtk_box_pack_start(GTK_BOX(vbox_center), ecu_speed_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_center), ecu_map_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_center), ecu_fuel_pressure_dial, TRUE, TRUE, 0);
   
   gtk_box_pack_start(GTK_BOX(vbox_right), ecu_fuel_tank_level_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_right), ecu_oil_temp_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_right), ecu_fuel_flow_dial, TRUE, TRUE, 0);

   /* Now pack the auxilliary gauges tab panel. */

   hbox_aux_top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); 
   gtk_widget_set_size_request (hbox_aux_top, 900, 10);
   hbox_aux_mid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
   hbox_aux_bot = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
   gtk_widget_set_size_request (hbox_aux_bot, 900, 10);
   
   hbox_aux_left = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); 
   gtk_widget_set_size_request (hbox_aux_left, 100, 500);
   hbox_aux_right = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); 
   gtk_widget_set_size_request (hbox_aux_right, 100, 500);


   gtk_box_pack_start(GTK_BOX(auxilliary_vbox), hbox_aux_top, TRUE, TRUE, 5); 
   gtk_box_pack_start(GTK_BOX(auxilliary_vbox), hbox_aux_mid, TRUE, TRUE, 5); 
   gtk_box_pack_start(GTK_BOX(auxilliary_vbox), hbox_aux_bot, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(hbox_aux_mid), hbox_aux_left, TRUE, TRUE, 5); 
   gtk_box_pack_start(GTK_BOX(hbox_aux_mid), aux_dial_grid, TRUE, TRUE, 5); 
   gtk_box_pack_start(GTK_BOX(hbox_aux_mid), hbox_aux_right, TRUE, TRUE, 5);

   /* does not work: gtk_box_set_center_widget (GTK_BOX(auxilliary_vbox), aux_dial_grid); */

   /* gtk_box_pack_start(GTK_BOX(auxilliary_vbox), hbox_aux_mid, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(auxilliary_vbox), hbox_aux_bot, TRUE, TRUE, 0); */

   /*
   gtk_box_pack_start(GTK_BOX(hbox_aux_top), aux_dial_1, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_aux_top), aux_dial_2, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_aux_top), aux_dial_3, TRUE, TRUE, 0);
   
   gtk_box_pack_start(GTK_BOX(hbox_aux_mid), aux_dial_4, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_aux_mid), aux_dial_5, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_aux_mid), aux_dial_6, TRUE, TRUE, 0);
   
   gtk_box_pack_start(GTK_BOX(hbox_aux_bot), aux_dial_7, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_aux_bot), aux_dial_8, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_aux_bot), aux_dial_9, TRUE, TRUE, 0);   */  
   

    
   /* TODO: Now pack the DTC tab panel. */
   /* TODO: Now pack the PID tab panel. */
   /* TODO: Now pack the performance tab panel. */
   /* Now pack the communications tab panel. */
   gtk_box_pack_start(GTK_BOX(communications_vbox), comms_text_frame, TRUE, TRUE, 0);
   
   /* Set up the callback functions. */
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);  

   gtk_widget_show_all(window);
   
   set_ecu_connected(0);
   int ecu_auto_connect = 0; /* TODO: add to configuration options: get_auto_connect(). */
   if (ecu_auto_connect == 1) 
   {
      memset(obd_protocol, 0, 256);
      if (argc < 2) /* Get protocol number from command line. */
      {
         strcpy(obd_protocol, "ATTP 0\r");
      }
      else
      {
         strncpy(obd_protocol, "ATTP %c\r", argv[1][0]);
      }
      
      if (server_connect() > 0) /* Sockets Module Connect Function. */
      {
         init_obd_comms(obd_protocol);
        
         send_ecu_msg("ATDP\n");  /* Get OBD protocol name from interface. */
         send_ecu_msg("ATRV\r");  /* Get battery voltage from interface. */
         send_ecu_msg("09 02\r"); /* Get vehicle VIN number. */
         send_ecu_msg("09 0A\r"); /* Get ECU name. */
         send_ecu_msg("01 01\r"); /* Get DTC Count and MIL status. */
         send_ecu_msg("01 00\r"); /* Get supported PIDs 1 - 32 for MODE 1. */
         send_ecu_msg("09 00\r"); /* Get supported PIDs 1 - 32 for MODE 9. */
         send_ecu_msg("03\r");
         
         g_timeout_add (60000, send_obd_message_60sec_callback, (gpointer)window);
         /* g_timeout_add (10000, send_obd_message_10sec_callback, (gpointer)window); */
         g_timeout_add (1000, send_obd_message_1sec_callback, (gpointer)window);
         g_timeout_add (100, recv_obd_message_callback, (gpointer)window);
         set_status_msg("Connected to ECU.");
  
      }
      else
      {
         set_status_msg("Connection to server failed.");
      }
   }


   gtk_main();  
   
   close_log_file();
   
   return 0;
}
