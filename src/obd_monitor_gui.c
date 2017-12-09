/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A UDP server that communicates with vehicle
                engine control units via an OBD-II interface to obtain 
                engine status and fault codes. 

                Implements two functions:

                1. A UDP datagram server that receives requests for vehicle
                status information from a client application (GUI) and 
                returns the requested information to the client. 

                2. Serial communications to request vehicle status
                information and fault codes from the engine control unit using 
                the OBD-II protocol.

   Date: 30/11/2017
   
*/


#include <gtk/gtk.h>
#include <math.h>
#include "obd_monitor.h"
#include "protocols.h"

int sock;
unsigned int length;
struct sockaddr_in obd_server, from;
struct hostent *hp;


/* Buffers for Engine Control Unit messages. */
char ECU_PID_Request[256];
char ECU_PID_Reply[256];

/* Buffers for OBD Interface messages. */
char ODB_Request[256];
char OBD_Reply[256];

/* Status, error and warning message buffers. */
char current_info_msg[256];
char current_error_msg[256];
char current_status_msg[256];
char current_warning_msg[256];
char current_question_msg[256];

/* Current time string. */
char time_buffer[256];


/* Function declarations. */
void show_error_msg(GtkWidget *widget, gpointer window);

/* ----------------------- */
/* Server comms functions. */
/* ----------------------- */

int init_server_comms(char *server, char *port)
{
   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) 
      printf("ERROR: socket creation failed.\n");

   obd_server.sin_family = AF_INET;
   hp = gethostbyname(server);
   if (hp == 0) 
      printf("ERROR: Unknown host -> %s.\n", server);

   bcopy((char *)hp->h_addr, (char *)&obd_server.sin_addr, hp->h_length);
   obd_server.sin_port = htons(atoi(port));
   length = sizeof(struct sockaddr_in);

   return(sock);
}

int send_ecu_msg(char *query)
{
   char buffer[256];
   int n;

   bzero(buffer,256);
   sprintf(buffer,"%s\n",query);

   n = sendto(sock,buffer,strlen(buffer),0,(const struct sockaddr *)&obd_server,length);
   if (n < 0) 
      printf("ERROR: Sendto failed.\n");
      
   printf("SENT ECU Message: %s\n", buffer);

   return n;
}

int recv_ecu_msg(char *ecu_data)
{
   char buffer[256];
   int n;

   bzero(buffer,256);

   n = recvfrom(sock,buffer,256,MSG_DONTWAIT,(struct sockaddr *)&from,&length);
   /* We are not blocking on recv now.
    if (n < 0) 
      printf("ERROR: recvfrom failed.\n"); */
   if (n > 0)
      printf("RECV ECU Message: %s\n", buffer);

   return n;
}


int init_obd_comms(char *obd_msg)
{
   int n;
   char buffer[256];

   bzero(buffer,256);
   sprintf(buffer,"%s\n",obd_msg); /* Require newline 0x0D terminator for all messages. */

   n = sendto(sock,buffer,strlen(buffer),0,(const struct sockaddr *)&obd_server,length);

   if (n < 0) 
      printf("ERROR: Sendto failed.\n");

   printf("SENT OBD Message: %s\n", buffer);

   usleep(OBD_WAIT_TIMEOUT);

   bzero(buffer,256);

   n = recvfrom(sock,buffer,256,MSG_DONTWAIT,(struct sockaddr *)&from,&length);
   
   if (n > 0)
      printf("RECV OBD Message: %s\n", buffer);   

   return(n);
}

/* ----------------------- */
/* GUI callback functions. */
/* ----------------------- */

void combo_selected(GtkComboBoxText *widget, gpointer window) 
{
  gchar *text = gtk_combo_box_text_get_active_text(widget);
  if (text != NULL)
  {
     g_printf("You chose %s\n", text);
  }
  g_free(text);
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


void send_query(GtkWidget *widget, gpointer window) 
{
   char buffer[256];

   bzero(buffer, 256);
   bzero(ECU_PID_Request, 16);
   sprintf(ECU_PID_Request,"0100");
   send_ecu_msg(ECU_PID_Request);
   /* g_printf("Sent ECU message: %s\n", ECU_PID_Request); */
   usleep(OBD_WAIT_TIMEOUT);
   recv_ecu_msg(buffer);

   return;
}

void ecu_connect(GtkWidget *widget, gpointer window) 
{
   int result;

   bzero(current_error_msg, 256);
   /* First set up UDP communication with the server process
      and check connection to the OBD interface. */
   result = init_server_comms("127.0.0.1", "8989");
   if (result < 0)
   {
      /* Pop up an error dialog. */
      strncpy(current_error_msg, "ERROR: Failed to connect to OBD server.\n", 40);
      show_error_msg(widget, window);
   }
   else
   {
      result = init_obd_comms("ATI");
      if (result <= 0)
      {
         strncpy(current_error_msg, "ERROR: Failed to connect to OBD interface.\n", 43);
         show_error_msg(widget, window);
      }
   }
   

   return;
}



static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
  cairo_move_to(cr, 30, 30);
  cairo_set_font_size(cr, 15);
  cairo_show_text(cr, time_buffer);

  return FALSE;
}

static void draw_dial_background(cairo_t *cr, double width, double height)
{
   /* */
   double x         = 5.0,                /* parameters like cairo_rectangle */
          y         = 5.0,
          aspect        = 1.0,             /* aspect ratio */
          corner_radius = height / 10.0;   /* and corner curvature radius */

   double radius = corner_radius / aspect;
   double degrees = NUM_PI / 180.0;

   /* Draw the background shapes. */
   
   cairo_new_sub_path (cr);
   cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
   cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
   cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
   cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
   cairo_close_path (cr);

   cairo_set_source_rgb (cr, 0.125, 0.29, 0.53);
   cairo_fill_preserve (cr);
   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);
   cairo_stroke (cr);
   
 
   
   return;
}

static void draw_dial_gauge(cairo_t *cr, double width, double height, double lower, double upper, double radius, double angle)
{
   /* Draw the gauge and needle */
   
   DialPoint points[6];
   double s,c;
   double theta, last, increment;
   double xc, yc;
   double tick_length;
   int i, inc;
   double pointer_width = 10.0;
  
   xc = width / 2.0;
   yc = height / 2.0;

   /* Draw ticks */

   if ((upper - lower) == 0)
     return;

   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width(cr, 2.0);
   
   increment = (100*NUM_PI) / (radius*radius);

   inc = (upper - lower);

   while (inc < 100) inc *= 10;
   while (inc >= 1000) inc /= 10;
   last = -1;

   for (i = 0; i <= inc; i++)
   {
      theta = ((double)i*NUM_PI / (18*inc/24.0) - NUM_PI/6.0);

      if ((theta - last) < (increment))
        continue;     
      last = theta;

      s = sin (theta);
      c = cos (theta);

      tick_length = (i%(inc/10) == 0) ? pointer_width : pointer_width / 2.0;

      cairo_move_to(cr, xc + c*(radius - tick_length), yc - s*(radius - tick_length));
      cairo_line_to(cr, xc + c*radius, yc - s*radius);
      
   }
   cairo_stroke(cr);

   /* Draw pointer */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 2.0);
   cairo_move_to(cr, xc, yc);
   
   s = sin (angle);
   c = cos (angle);

   points[0].x = xc + s*pointer_width/2;
   points[0].y = yc + c*pointer_width/2;
   points[1].x = xc + c*radius;
   points[1].y = yc - s*radius;
   points[2].x = xc - s*pointer_width/2;
   points[2].y = yc - c*pointer_width/2;
   points[3].x = xc - c*radius/10;
   points[3].y = yc + s*radius/10;
   points[4].x = points[0].x;
   points[4].y = points[0].y;


   for(i = 0; i < 4; i++)
   {
     cairo_move_to(cr, points[i].x, points[i].y);
     cairo_line_to(cr, points[i+1].x, points[i+1].y);
   }
   

   cairo_stroke(cr);
    
   return;
}

static void fill_dial_background(cairo_t *cr)
{
   /* a custom shape that could be wrapped in a function */
   double x         = 5.0,                /* parameters like cairo_rectangle */
          y         = 5.0,
          width         = 190,
          height        = 140,
          aspect        = 1.0,             /* aspect ratio */
          corner_radius = height / 10.0;   /* and corner curvature radius */

   double radius = corner_radius / aspect;
   double degrees = NUM_PI / 180.0;

   cairo_new_sub_path (cr);
   cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
   cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
   cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
   cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
   cairo_close_path (cr);

   cairo_set_source_rgb (cr, 0.125, 0.29, 0.53);
   cairo_fill_preserve (cr);
   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);
   cairo_stroke (cr);
   
   return;
}

static gboolean draw_rpm_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   double rpm_scale_factor = 38.2; /* scale factor = max rpm on gauge / arc length of gauge.     */
                                   /* arc length = 2 * PI * radius * theta/360 if angle is in degrees. */
                                   /* arc length = radius * theta  if the angle is in radians.   */
                                   
 
   /* double dial_start_angle = 0.25 * NUM_PI;  45 degrees */
   /* double dial_end_angle = -1.25 * NUM_PI;   225 degrees */
   
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI; /* 210 degrees */

   cairo_text_extents_t ctext;
   
   draw_dial_background(cr, 190, 140);
   /* draw_dial_gauge(cr, 200, 150, 0, 7000, radius, 45); */

   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);   
   cairo_arc_negative(cr, xc, yc, radius, gauge_start_angle, gauge_end_angle);
   cairo_stroke(cr);

   /* Draw pointer. */
   
   double engine_rpm = 6000; /* TODO: get rpm value from protocol module. */
                             /* NOTE: ensure rpm value is greater than zero */
   double gauge_rpm = engine_rpm / rpm_scale_factor; /* this is the gauge arc length for the needle. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_rpm / radius); /* Angle in radians. */
   printf("Needle angle and arc len: %f - %f - \n", needle_angle, gauge_rpm/radius);
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI); /* Center dot. */
   cairo_fill(cr);

   cairo_arc(cr, xc, yc, radius, needle_angle, needle_angle); /* Needle. */
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"Engine RPM",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "Engine RPM");
  
   return TRUE;
}


static gboolean draw_speed_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   /* cairo_set_line_width (cr, 5.0);
   cairo_rectangle (cr, 0, 0, 200, 150);
   cairo_stroke(cr); */
   
   fill_dial_background(cr);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);

   /* cairo_arc (cr, xc, yc, radius, angle1, angle1); 
   cairo_line_to (cr, xc, yc); */
   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"Speedometer",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "Speedometer");
  
   return TRUE;
}

static gboolean draw_ect_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   /* cairo_set_line_width (cr, 5.0);
   cairo_rectangle (cr, 0, 0, 200, 150);
   cairo_stroke(cr); */
   
   fill_dial_background(cr);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);

   /* cairo_arc (cr, xc, yc, radius, angle1, angle1); 
   cairo_line_to (cr, xc, yc); */
   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"Coolant Temp",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "Coolant Temp");
  
   return FALSE;
}

static gboolean draw_iat_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   /* cairo_set_line_width (cr, 5.0);
   cairo_rectangle (cr, 0, 0, 200, 150);
   cairo_stroke(cr); */
   
   fill_dial_background(cr);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);

   /* cairo_arc (cr, xc, yc, radius, angle1, angle1); 
   cairo_line_to (cr, xc, yc); */
   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"Intake Air Temp",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "Intake Air Temp");
  
   return FALSE;
}

static gboolean draw_map_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   /* cairo_set_line_width (cr, 5.0);
   cairo_rectangle (cr, 0, 0, 200, 150);
   cairo_stroke(cr); */
   
   fill_dial_background(cr);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);

   /* cairo_arc (cr, xc, yc, radius, angle1, angle1); 
   cairo_line_to (cr, xc, yc); */
   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"Manifold Pressure",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "Manifold Pressure");
  
   return FALSE;
}

static gboolean draw_egr_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   fill_dial_background(cr);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);

   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"EGR Pressure",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "EGR Pressure");
  
   return FALSE;
}

static gboolean draw_oil_pressure_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   fill_dial_background(cr);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);

   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"Oil Pressure",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "Oil Pressure");
  
   return FALSE;
}


static gboolean draw_oil_temp_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   fill_dial_background(cr);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);
   
   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"Oil Temperature",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "Oil Temperature");
  
   return FALSE;
}

static gboolean draw_fuel_flow_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   fill_dial_background(cr);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);

   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"Fuel Flow Rate",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "Fuel Flow Rate");
  
   return FALSE;
}


static void fill_large_dial_background(cairo_t *cr)
{
   /* a custom shape that could be wrapped in a function */
   double x         = 5.0,                /* parameters like cairo_rectangle */
          y         = 5.0,
          width         = 290,
          height        = 220,
          aspect        = 1.0,             /* aspect ratio */
          corner_radius = height / 10.0;   /* and corner curvature radius */

   double radius = corner_radius / aspect;
   double degrees = NUM_PI / 180.0;

   cairo_new_sub_path (cr);
   cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
   cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
   cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
   cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
   cairo_close_path (cr);

   cairo_set_source_rgb (cr, 0.125, 0.29, 0.53);
   cairo_fill_preserve (cr);
   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);
   cairo_stroke (cr);
   
   return;
}

static gboolean draw_pid_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   cairo_text_extents_t ctext;
   
   fill_large_dial_background(cr);

   cairo_select_font_face (cr, "cairo :monospace",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
                           
   cairo_text_extents (cr,"PID",&ctext);
   printf("Text width: %f\n", ctext.width);
   printf("Text start: %f\n", (150.0 - (0.5 * ctext.width)));
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (135.0 - (0.5 * ctext.width)), 210);
   cairo_set_font_size(cr, 32);
   cairo_show_text(cr, "PID");
  
   return FALSE;
}

static gboolean draw_dtc_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   cairo_text_extents_t ctext;
   
   fill_large_dial_background(cr);
   cairo_select_font_face (cr, "cairo :monospace",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
   cairo_text_extents (cr,"DTC",&ctext);
   printf("Text width: %f\n", ctext.width);
   printf("Text start: %f\n", (150.0 - (0.5 * ctext.width)));
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (125.0 - (0.5 * ctext.width)), 210);
   cairo_set_font_size(cr, 32);
   cairo_show_text(cr, "DTC");
  
   return FALSE;
}

static void fill_small_dial_background(cairo_t *cr)
{
   /* a custom shape that could be wrapped in a function */
   double x         = 5.0,                /* parameters like cairo_rectangle */
          y         = 5.0,
          width         = 290,
          height        = 30,
          aspect        = 1.0,             /* aspect ratio */
          corner_radius = height / 2.0;   /* and corner curvature radius */

   double radius = corner_radius / aspect;
   double degrees = NUM_PI / 180.0;

   cairo_new_sub_path (cr);
   cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 90 * degrees);
   /* cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees); */
   cairo_arc (cr, x + radius, y + radius, radius, 90 * degrees, 270 * degrees);
   /* cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees); */
   cairo_close_path (cr);

   cairo_set_source_rgb (cr, 0.125, 0.29, 0.53);
   cairo_fill_preserve (cr);
   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);
   cairo_stroke (cr);
   
   return;
}

static gboolean draw_battery_voltage_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 10.0;
   double yc = 25.0;
   cairo_text_extents_t ctext;
   
   fill_small_dial_background(cr);

   cairo_text_extents (cr,"Battery Voltage: 12.4V",&ctext);
   printf("Text width: %f\n", ctext.width);
   printf("Text start: %f\n", (120.0 - (0.5 * ctext.width)));
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (120.0 - (0.5 * ctext.width)), yc);
   cairo_set_font_size(cr, 16);
   cairo_show_text(cr, "Battery Voltage: 12.4V");
  
   return FALSE;
}


gboolean time_handler(GtkWidget *widget) 
{
    
  if (widget == NULL) return FALSE;

  GDateTime *now = g_date_time_new_now_local(); 
  gchar *my_time = g_date_time_format(now, "%H:%M:%S");
  
  g_sprintf(time_buffer, "%s", my_time);
  
  g_free(my_time);
  g_date_time_unref(now);

  gtk_widget_queue_draw(widget);
  
  return TRUE;
}


/* What the hell is this for??? */
void on_changed (GtkComboBoxText *widget, gpointer user_data)
{
  GtkComboBoxText *combo_box = widget;
  gchar *obd_protocol;

  obd_protocol = gtk_combo_box_text_get_active_text(combo_box);

  if (obd_protocol != NULL) 
  {
    g_printf("You chose %s\n", obd_protocol);
  }
  g_free (obd_protocol);

  return;
}


void show_info(GtkWidget *widget, gpointer window) 
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

void show_error_msg(GtkWidget *widget, gpointer window) 
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

void show_error(GtkWidget *widget, gpointer window) 
{
    
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Error loading file");
  gtk_window_set_title(GTK_WINDOW(dialog), "Error");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  return;
}

void show_question(GtkWidget *widget, gpointer window) 
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

void show_warning(GtkWidget *widget, gpointer window) 
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

int main(int argc, char *argv[]) 
{
   GtkWidget *window;
   GdkPixbuf *icon;

   GtkWidget *dtc_button;
   GtkWidget *egr_button;
   GtkWidget *iat_button;
   GtkWidget *map_button;
   GtkWidget *pid_button;
   GtkWidget *ecu_request_button;
   GtkWidget *ecu_connect_button;

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
   GtkWidget *battery_voltage_dial;
   
   GtkWidget *dtc_dial;
   GtkWidget *pid_dial;

   GtkWidget *text_view;
   GtkTextBuffer *text_buffer;
   GtkWidget *text_frame;
   GtkWidget *scrolled_window;
   

   gtk_init(&argc, &argv);

   g_printf("GTK+ version: %d.%d.%d\n", gtk_major_version, gtk_minor_version, gtk_micro_version);
   g_printf("Glib version: %d.%d.%d\n", glib_major_version, glib_minor_version, glib_micro_version);    


   /* Set up the main window. */
   window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "OBD-II Monitor");
   gtk_window_set_default_size(GTK_WINDOW(window), 1000, 600);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 10);

   icon = create_pixbuf("../images/setroubleshoot_red_icon.svg");  
   gtk_window_set_icon(GTK_WINDOW(window), icon);

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
   g_signal_connect(dtc_button, "clicked", G_CALLBACK(show_info), NULL); 
   gtk_widget_set_tooltip_text(dtc_button, "Button widget");

   iat_button = gtk_button_new_with_mnemonic("_IAT");
   g_signal_connect(iat_button, "clicked", G_CALLBACK(show_error), NULL); 
   gtk_widget_set_tooltip_text(iat_button, "Button widget");

   map_button = gtk_button_new_with_mnemonic("_MAP");
   g_signal_connect(map_button, "clicked", G_CALLBACK(show_warning), NULL); 
   gtk_widget_set_tooltip_text(map_button, "Button widget");

   egr_button = gtk_button_new_with_mnemonic("_TEG");
   g_signal_connect(egr_button, "clicked", G_CALLBACK(show_question), NULL); 
   gtk_widget_set_tooltip_text(egr_button, "Button widget");

   pid_button = gtk_button_new_with_mnemonic("_PID Lookup");
   g_signal_connect(pid_button, "clicked", G_CALLBACK(show_question), NULL); 
   gtk_widget_set_tooltip_text(pid_button, "Button widget");

   ecu_request_button = gtk_button_new_with_mnemonic("ECU _Request");
   g_signal_connect(ecu_request_button, "clicked", G_CALLBACK(send_query), NULL); 
   gtk_widget_set_tooltip_text(ecu_request_button, "Button widget");

   ecu_connect_button = gtk_button_new_with_mnemonic("_ECU Connect");
   g_signal_connect(ecu_connect_button, "clicked", G_CALLBACK(ecu_connect), NULL); 
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
   ecu_egr_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_egr_dial, 200, 150);
   g_signal_connect(ecu_egr_dial, "draw", G_CALLBACK(draw_egr_dial), NULL);
   ecu_oil_temp_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_oil_temp_dial, 200, 150);
   g_signal_connect(ecu_oil_temp_dial, "draw", G_CALLBACK(draw_oil_temp_dial), NULL);
   ecu_oil_pressure_dial = gtk_drawing_area_new();
   gtk_widget_set_size_request (ecu_oil_pressure_dial, 200, 150);
   g_signal_connect(ecu_oil_pressure_dial, "draw", G_CALLBACK(draw_oil_pressure_dial), NULL);
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

   /* Set up other widgets. */

   protocol_combo_box = gtk_combo_box_text_new();
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "0 - Automatic OBD-II Protocol Search");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "1 - SAE J1850 PWM (41.6 kbaud)(Ford)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "2 - SAE J1850 VPW (10.4 kbaud)(GM, Isuzu)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "3 - IS0 9141-2 (5 baud init, 10.4 kbaud)(Chrysler)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "4 - ISO 14230-4 KWP2000 (5-baud init.)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "5 - IS0 14230-4 KWP2000 (Fast init.)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "6 - IS0 15765-4 CAN (11 bit ID, 500 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "7 - IS0 15765-4 CAN (29 bit ID, 500 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "8 - IS0 15765-4 CAN (11 bit ID, 250 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "9 - IS0 15765-4 CAN (29 bit ID, 250 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "A - SAE J1939 CAN (29 bit ID, 250 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "B - USER1 CAN (11 bit ID, 125 kbaud)");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(protocol_combo_box), NULL, "C - USER2 CAN (11 bit ID, 50 kbaud)");
   g_signal_connect(protocol_combo_box, "changed", G_CALLBACK(combo_selected), NULL);

   text_view = gtk_text_view_new ();
   text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
   gtk_widget_set_size_request (text_view, 880, 80);
   gtk_text_buffer_set_text (text_buffer, "ATI\nOK\n>\nATZ\nOK\n>\n", -1);
   text_frame = gtk_frame_new("Communications Log");
   scrolled_window = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
   gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);

   /* Set up labels. */   
   combo_label = gtk_label_new (NULL);
   gtk_label_set_markup (GTK_LABEL (combo_label), "<b>OBD-II Protocol: </b>");
   instruments_label = gtk_label_new (NULL);
   gtk_label_set_markup (GTK_LABEL (instruments_label), "<b>Instruments: </b>");
   text_view_label = gtk_label_new(NULL);
   gtk_label_set_markup (GTK_LABEL (text_view_label), "Communications Log");
   

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
   gtk_box_pack_start(GTK_BOX(vbox), hbox_top, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(vbox), hbox_center, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(vbox), hbox_bottom, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_top), combo_label, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_top), protocol_combo_box, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_top), ecu_connect_button, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_top), ecu_request_button, TRUE, TRUE, 0);
   /* gtk_box_pack_start(GTK_BOX(hbox_top), dtc_button, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox_top), pid_button, TRUE, TRUE, 0); */
   gtk_box_pack_start(GTK_BOX(hbox_top), battery_voltage_dial, TRUE, TRUE, 0);
   gtk_container_add (GTK_CONTAINER (text_frame), scrolled_window);
   gtk_box_pack_start(GTK_BOX(hbox_bottom), text_frame, TRUE, TRUE, 0);
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
   gtk_box_pack_start(GTK_BOX(vbox_center), ecu_egr_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_right), ecu_oil_pressure_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_right), ecu_oil_temp_dial, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(vbox_right), ecu_fuel_flow_dial, TRUE, TRUE, 0);


   /* Set up the callback functions. */
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);  

   gtk_widget_show_all(window);
   time_handler(window);
   /* g_object_unref(icon); */

   gtk_main();  
      
   return 0;
}
