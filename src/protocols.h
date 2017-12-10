/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: OBD and AT message crackers. 
                If the message starts with ASCII hex digits '40'...'49' then
                the message is from the ECU. Otherwise it is a message from
                the OBD interface IC or is invalid.

   Date: 7/11/2017
   
*/


/* Type Definitions. */

struct _ECU_Parameters {
   double ecu_engine_rpm;
   double ecu_vehicle_speed;
   double ecu_coolant_temperature;
   double ecu_intake_air_temperature;
   double ecu_manifold_air_pressure;
   double ecu_oil_pressure;
   double ecu_egr_pressure;
   double ecu_battery_voltage;
   char ecu_vin[256];
};

typedef struct _ECU_Parameters ECU_Parameters;

struct _OBD_Interface {
   unsigned int obd_interface_status;
   unsigned int obd_rs232_baud;
   unsigned int obd_stop_bits;
   unsigned int obd_data_bits;
   unsigned int obd_parity_bits;
   int obd_protocol_number;
   char obd_protocol_name[256];
};

typedef struct _OBD_Interface OBD_Interface;

/* Function Declarations. */

/* OBD Interface Status. */
void set_interface_on();
void set_interface_off();
int get_interface_status();

/* ECU Parameter Get/Set Functions. */

void set_ecu_parameters(ECU_Parameters *ecup);
void get_ecu_parameters(ECU_Parameters *ecup);

void set_engine_rpm(double rpm);
double get_engine_rpm();

void set_coolant_temperature(double ctemp);
double get_coolant_temperature();

void set_manifold_pressure(double manap);
double get_manifold_pressure();

void set_intake_air_temperature(double atemp);
double get_intake_air_temperature();

void set_battery_voltage(double bv);
double get_battery_voltage();

void set_vehicle_speed(double vs);
double get_vehicle_speed();

void set_egr_pressure(double egrp);
double get_egr_pressure();

void set_oil_temperature(double otemp);
double get_oil_temperature();

void set_oil_pressure(double oilp);
double get_oil_pressure();

/* Message Parsers. */
int parse_obd_msg(char *obd_msg);

