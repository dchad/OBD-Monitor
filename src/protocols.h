/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: OBD and AT message crackers. 
                If the message starts with ASCII hex digits '40'...'49' then
                the message is from the ECU. Otherwise it is a message from
                the OBD interface IC or is invalid.

   Date: 7/11/2017
   
*/

/* Function Declarations. */

/* OBD Interface Status. */
void set_interface_on();
void set_interface_off();
int get_interface_status();

/* ECU Parameter Get/Set Functions. */
void set_engine_rpm(double rpm);
double get_engine_rpm();

/* Message Parsers. */
int parse_obd_msg(char *obd_msg);

