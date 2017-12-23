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


/* Current time string. */
char time_buffer[256];
char status_bar_msg[256];

/* Communications log display area. */
GtkTextBuffer *text_buffer;
GtkTextIter text_iter;

/* ----------------------- */
/* GUI callback functions. */
/* ----------------------- */

void update_comms_log_view(char *msg)
{
   gtk_text_buffer_get_iter_at_offset(text_buffer, &text_iter, -1);
   gtk_text_buffer_insert(text_buffer, &text_iter, msg, -1);
   
   return;
}

void set_status_bar_msg(char *msg)
{
   memset(status_bar_msg, 0, 256);
   if (strlen(msg) > 0)
   {
      strncpy(status_bar_msg, msg, strlen(msg));
      g_strchomp(status_bar_msg); /* Remove trailing whitespace. */
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
  char msg_buf[256];
  int selected;
  
  memset(msg_buf, 0, 256);
  
  if (text != NULL)
  {
     selected = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
     printf("protocol_combo_selected() : Protocol Selected: %i %s\n", selected, text);
     sprintf(msg_buf, "ATSP %.2x\n", selected);
     send_ecu_msg(msg_buf);
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
   g_printf("Button clicked\n");

   return;
}

gint send_obd_message_60sec_callback (gpointer data)
{
   send_ecu_msg("ATRV\n"); /* Battery Voltage */
   
   /* gtk_widget_queue_draw((GtkWidget *)data); */
   
   return(TRUE);
}

gint send_obd_message_30sec_callback (gpointer data)
{
   send_ecu_msg("01 05\n"); /* Coolant Temperature */
   send_ecu_msg("01 2F\n"); /* Fuel Tank Level */
   send_ecu_msg("01 0F\n"); /* Intake Air Temperature */
   send_ecu_msg("01 5C\n"); /* Oil Temperature */
    
   return(TRUE);
}

gint send_obd_message_10sec_callback (gpointer data)
{
   send_ecu_msg("01 05\n"); /* Coolant Temperature */
   send_ecu_msg("01 2F\n"); /* Fuel Tank Level */
   send_ecu_msg("01 0F\n"); /* Intake Air Temperature */
   send_ecu_msg("01 5C\n"); /* Oil Temperature */
   
   return(TRUE);
}

gint send_obd_message_1sec_callback (gpointer data)
{

   send_ecu_msg("01 0C\n"); /* Engine RPM */
   send_ecu_msg("01 0D\n"); /* Vehicle Speed */
   send_ecu_msg("01 0A\n"); /* Fuel Pressure */
   send_ecu_msg("01 0B\n"); /* MAP Pressure */
   send_ecu_msg("01 5E\n"); /* Fuel Flow Rate */
   
   return(TRUE);
}

gint recv_obd_message_callback (gpointer data)
{
   recv_ecu_msg();
   gtk_widget_queue_draw((GtkWidget *)data);
   
   return(TRUE);
}

void ecu_connect_callback(GtkWidget *widget, gpointer window) 
{
   int result;

   if (get_ecu_connected() == 1)
   {
      return;
   }
   if (ecu_connect() > 0)
   {
      set_ecu_connected(1); /* Start PID comms with server process. */
      g_timeout_add (60000, send_obd_message_60sec_callback, (gpointer)window);
      g_timeout_add (1000, send_obd_message_1sec_callback, (gpointer)window);
      g_timeout_add (100, recv_obd_message_callback, (gpointer)window);  
   }
   else
   {
      /* TODO: popup error dialog. */
   }

   return;
}


gboolean image_press_callback(GtkWidget *event_box, GdkEventButton *event, gpointer data)
{
   g_print ("Event box clicked at coordinates %f,%f\n", event->x, event->y);

   // Returning TRUE means we handled the event, so the signal
   // emission should be stopped (don’t call any further callbacks
   // that may be connected). Return FALSE to continue invoking callbacks.
  
   return TRUE;
}

GtkWidget* create_image(char *image_file, GtkWidget *cbox)
{
  GtkWidget *image;
  GtkWidget *event_box;
  GtkWidget *info_label, *notice_label;
  GtkWidget *align;


  image = gtk_image_new_from_file (image_file);
  info_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (info_label), "<b>THIS FUNCTION NOT IMPLEMENTED YET SAD FACE!</b>");
  notice_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (notice_label), "<b>INFORMATIONAL SAD FACE</b>");

  event_box = gtk_event_box_new ();

  align = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
  
  gtk_container_add (GTK_CONTAINER (event_box), image);
  gtk_container_add (GTK_CONTAINER (align), event_box);
  gtk_box_pack_start (GTK_BOX (cbox), notice_label, FALSE, FALSE, 10);
  gtk_box_pack_start (GTK_BOX (cbox), align, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(cbox), info_label, FALSE, FALSE, 10);

  g_signal_connect (G_OBJECT (event_box),
                    "button_press_event",
                    G_CALLBACK (image_press_callback),
                    image);

  /* gtk_box_pack_start(GTK_BOX(cbox), event_box, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(cbox), info_label, FALSE, FALSE, 10); */

  return image;
}

GtkWidget *create_tab_panel(GtkWidget *window, GtkWidget *instruments_vbox, GtkWidget *communications_vbox, 
                            GtkWidget *pid_vbox, GtkWidget *dtc_vbox, GtkWidget *performance_vbox)
{
   GtkWidget *notebook;
   GtkWidget *tab_instruments_label;
   GtkWidget *tab_communications_label;
   GtkWidget *tab_PID_label;
   GtkWidget *tab_DTC_label;
   GtkWidget *tab_performance_label;
   GtkWidget *image_sad;
   GtkWidget *image_happy;


   image_sad = create_image("../images/face-sad.png", pid_vbox);
   image_sad = create_image("../images/face-sad.png", dtc_vbox);
   image_sad = create_image("../images/face-sad.png", performance_vbox);
    
   
   notebook = gtk_notebook_new ();
   gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
   
   tab_instruments_label = gtk_label_new ("Instruments");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), instruments_vbox, tab_instruments_label);
	
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


int main(int argc, char *argv[]) 
{
   GtkWidget *window;
   GtkWidget *tab_panel;

   GtkWidget *dtc_button;
   GtkWidget *egr_button;
   GtkWidget *iat_button;
   GtkWidget *map_button;
   GtkWidget *pid_button;
   GtkWidget *ecu_request_button;
   GtkWidget *ecu_connect_button;

   GtkWidget *instruments_vbox;
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
   
   GtkWidget *dtc_dial;
   GtkWidget *pid_dial;

   GtkWidget *text_view;
   GtkWidget *text_frame;
   GtkWidget *comms_scrolled_window;
   
   GtkWidget *status_frame;
   
   FILE *logfile;
   
   logfile = open_log_file("./", "obd_gui_log.txt");

   gtk_init(&argc, &argv);

   g_printf("GTK+ version: %d.%d.%d\n", gtk_major_version, gtk_minor_version, gtk_micro_version);
   g_printf("Glib version: %d.%d.%d\n", glib_major_version, glib_minor_version, glib_micro_version);    


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
   text_view = gtk_text_view_new ();
   text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
   gtk_widget_set_size_request (text_view, 880, 600);
   gtk_text_buffer_get_iter_at_offset(text_buffer, &text_iter, 0);
   gtk_text_buffer_insert(text_buffer, &text_iter, "OBD Monitor Initialising...\n", -1);
   text_frame = gtk_frame_new("Communications Log");
   comms_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (comms_scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
   gtk_container_add (GTK_CONTAINER (comms_scrolled_window), text_view);


   status_frame = gtk_frame_new("Nofifications");
  
  
   /* Set up labels. */   
   combo_label = gtk_label_new (NULL);
   gtk_label_set_markup (GTK_LABEL (combo_label), "<b>OBD-II Protocol: </b>");
   instruments_label = gtk_label_new (NULL);
   gtk_label_set_markup (GTK_LABEL (instruments_label), "<b>Instruments: </b>");
   text_view_label = gtk_label_new(NULL);
   gtk_label_set_markup (GTK_LABEL (text_view_label), "<b>Communications Log</b>");
   
   /* Create the tab panels. */
   instruments_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   communications_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   pid_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   dtc_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   performance_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   tab_panel = create_tab_panel(window, instruments_vbox, communications_vbox, pid_vbox, dtc_vbox, performance_vbox);
   
   /* gtk_container_add(GTK_CONTAINER(window), vbox); */

   /* Set up the main window container layout. */
   vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   vbox_left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   vbox_center = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   vbox_right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   hbox_top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
   hbox_center = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
   hbox_bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
   vbox_controls = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   
   
   gtk_container_add(GTK_CONTAINER(window), vbox); 
   
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

   /* TODO: Now pack the DTC tab panel. */
   /* TODO: Now pack the PID tab panel. */
   /* TODO: Now packe the performance tab panel. */
   /* Now pack the communications tab panel. */
   gtk_box_pack_start(GTK_BOX(communications_vbox), comms_scrolled_window, TRUE, TRUE, 0);
   
   /* Set up the callback functions. */
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);  

   gtk_widget_show_all(window);
   
   set_ecu_connected(0);
   int ecu_auto_connect = 1; /* TODO: add to configuration options. */
   if (ecu_auto_connect == 1) 
   {
      if (ecu_connect() > 0) /* Sockets Module Connect Function. */
      {
         send_ecu_msg("ATDP\n"); /* Get OBD protocol name from interface. */
         send_ecu_msg("ATRV\n"); /* Get battery voltage from interface. */
         /* g_timeout_add (60000, send_obd_message_60sec_callback, (gpointer)window); */
         g_timeout_add (10000, send_obd_message_10sec_callback, (gpointer)window);
         g_timeout_add (1000, send_obd_message_1sec_callback, (gpointer)window);
         g_timeout_add (100, recv_obd_message_callback, (gpointer)window);
         set_status_msg("Connected to ECU.");
      }
      else
      {
         set_status_msg("Connection to ECU failed.");
      }
   }

   
   /* g_object_unref(icon); */

   gtk_main();  
   
   fclose(logfile);
   
   return 0;
}
