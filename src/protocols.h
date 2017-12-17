/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: OBD and AT message crackers. 
                If the message starts with ASCII hex digits '40'...'49' then
                the message is from the ECU. Otherwise it is a message from
                the OBD interface IC or is invalid.


   Selected ECU Mode 01 Parameters: 
   
   [PID] [Data Bytes] [Min Value] [Max Value] [Formula]           [Description]
    05    1            -40         215         A - 40             (ECT Centigrade)
    0A    1            0           765         3 * A              (Fuel Pressure kPa)
    0B    1            0           255         A                  (MAP Pressure kPa)
    0C    2            0           16,383.75   (256 * A + B) / 4  (Engine RPM)
    0D    1            0           255         A                  (Vehicle Speed)
    0E    1            -64         63.5        (A / 2) - 64       (Timing Advance: degrees before TDC) 
    0F    1            -40         215         A - 40             (IAT Centigrade)
    11    1            0           100         100 / 255 * A      (Throttle Position %)
    2F    1            0           100         100 / 255 * A      (Fuel Tank Level %)
    5A    1            0           100         100 / 255 * A      (Relative Accelerator Pedal Position %)
    5C    1            -40         215         A - 40             (Oil Temperature)
    5E    2            0           3276.75     (256 * A + B) / 20 (Fuel Flow Rate L/h)



   Selected ECU Mode 09 Parameters:
 
   [PID] [Data Bytes] [Description] 
    02    17           VIN - Vehicle Identification Number
    0A    20           ECU Name
         
     
   Selected ECU Mode 21 Parameters (Toyota):
   
       
                     
   Selected ECU Mode 22 Parameters (GM/Isuzu):
        
    (Oil Pressure? Mode: 22 PID: 115C)
    
    
    
   Date: 7/11/2017
   
*/

/* Constant Definitions. */

#define ECU_ECT_TEMPERATURE_MAX 215.0
#define ECU_ECT_TEMPERATURE_IN -40.0 
#define ECU_VEHICLE_SPEED_MAX 200.0
#define ECU_VEHICLE_SPEED_MIN 0.0
#define ECU_IAT_TEMPERATURE_MAX 215.0
#define ECU_IAT_TEMPERATURE_MIN -40.0
#define ECU_ENGINE_RPM_MAX 8000.0
#define ECU_ENGINE_RPM_MIN 0.0
#define ECU_MAP_PRESSURE_MAX 255.0
#define ECU_MAP_PRESSURE_MIN 0.0

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
   double ecu_throttle_position;
   double ecu_oil_temperature;
   double ecu_accelerator_position;
   double ecu_fuel_pressure;
   double ecu_fuel_flow_rate;
   double ecu_fuel_tank_level;
   double ecu_timing_advance;
   char ecu_vin[256];
   char ecu_name[256];
   char battery_voltage[256];
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

void set_engine_rpm(char *rpm_msg);
double get_engine_rpm();

void set_coolant_temperature(char *ctemp_msg);
double get_coolant_temperature();

void set_manifold_pressure(char *manap_msg);
double get_manifold_pressure();

void set_intake_air_temperature(char *atemp_msg);
double get_intake_air_temperature();

void set_battery_voltage(char *bv_msg);
double get_battery_voltage();

void set_vehicle_speed(char *vs_msg);
double get_vehicle_speed();

void set_egr_pressure(char *egrp_msg);
double get_egr_pressure();

void set_oil_temperature(char *otemp_msg);
double get_oil_temperature();

void set_oil_pressure(char *oilp_msg);
double get_oil_pressure();

void set_throttle_position(char *tp_msg);
double get_throttle_position();

/* Message Parsers. */
int parse_obd_msg(char *obd_msg);

