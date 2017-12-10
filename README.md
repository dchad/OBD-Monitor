# OBD-Monitor
Tools for interfacing with vehicle engine control units using the OBD-II protocol.


## STATUS: Experimental.

## 1. Introduction

 The On-Board Diagnostics II (OBD-II) standard is a mandatory requirement for 
 all vehicles manufactured since 1997 in many countries. It specifies a number 
 of protocols that manufacturers must implement for diagnostic scan tools to 
 communicate with engine control units (ECU). These protocols are used for 
 monitoring and troubleshooting vehicle drive trains, chassis and electrical 
 systems. Implementing a hardware interface is relatively easy with a basic 
 knowledge of electronics and one of the OBD interpreter kits available from 
 popular electronics stores (See section 5).


## 2. Design

  TODO:

## 3. User Interface

  TODO:

!["GUI"](https://github.com/dchad/OBD-Monitor/blob/master/images/gui-prototype.png "GUI Prototype")


## 4. OBD-II Protocol

### 4.1 OSI and SAE Standards

 1 - SAE J1850 PWM (41.6 kbaud)(Ford)
 
 2 - SAE J1850 VPW (10.4 kbaud)(GM, Isuzu)
 
 3 - IS0 9141-2 (5 baud init, 10.4 kbaud)(Chrysler)
 
 4 - ISO 14230-4 KWP2000 (5-baud init.)
 
 5 - IS0 14230-4 KWP2000 (Fast init.)
 
 6 - IS0 15765-4 CAN (11 bit ID, 500 kbaud)
 
 7 - IS0 15765-4 CAN (29 bit ID, 500 kbaud)
 
 8 - IS0 15765-4 CAN (11 bit ID, 250 kbaud)
 
 9 - IS0 15765-4 CAN (29 bit ID, 250 kbaud)
 
 A - SAE J1939 CAN (29 bit ID, 250 kbaud)
 
 B - USER1 CAN (11 bit ID, 125 kbaud)
 
 C - USER2 CAN (11 bit ID, 50 kbaud)
 
 Note: From 2008 Controller Area Network (CAN-Bus) protocols are mandatory.
 
 
### 4.2 Diagnostic Test Modes

The SAE J1979 standard currently defines ten possible diagnostic test modes:

 01 - show current data
 
 02 - show freeze frame data
 
 03 - show diagnostic trouble codes
 
 04 - clear trouble codes and stored values
 
 05 - test results, oxygen sensors
 
 06 - test results, non-continuously monitored
 
 07 - show pending trouble codes
 
 08 - special control mode
 
 09 - request vehicle information
 
 0A - request permanent trouble codes


### 4.2 Diagnostic Trouble Codes (DTC)

  TODO:

### 4.3 Parameter Identifiers (PID)

  TODO:
  
#### Selected ECU Mode 01 Parameters: 
   
   [PID] [Data Bytes] [Min Value] [Max Value] [Formula]           [Description]
   
    05    1            -40         215         A - 40             (ECT Centigrade)
    
    0B    1            0           255         A                  (MAP Pressure kPa)
    
    0C    2            0           16,383.75   (256 * A + B) / 4  (Engine RPM)
    
    0D    1            0           255         A                  (Vehicle Speed) 
    
    0F    1            -40         215         A - 40             (IAT Centigrade)
    
    11    1            0           100         100 / 256 * A      (Throttle Position %)
    
    5C    1            -40         215         A - 40             (Oil Temperature)
    
    5E    2            0           3276.75     (256 * A + B) / 20 (Fuel Flow Rate L/h)
    
    (Oil Pressure?)
    
    (EGR Pressure?)
    
    (Accelerator Position)

#### Selected ECU Mode 09 Parameters:
 
   [PID] [Data Bytes] [Description] 
   
    02    17           VIN - Vehicle Identification Number
    
    0A    20           ECU Name
 
## 5. Hardware Interfaces
 
### 5.1 Integrated Circuits and Projects
 
  1. ELM327 OBD Interpreter - https://www.elmelectronics.com/wp-content/uploads/2016/07/ELM327DS.pdf
  2. STN1110 OBD Interpreter - http://www.obdsol.com/solutions/chips/stn1110/
  3. Sparkfun Car Diagnostics Kit - https://www.sparkfun.com/products/10769
  4. Altronics OBD Interpreter Kit - http://www.altronics.com.au/p/k4065-obdii-automotive-interpreter-kit/

### 5.2 Pics


## 6. Notes
