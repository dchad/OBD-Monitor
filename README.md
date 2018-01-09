# OBD-Monitor
Tools for interfacing with vehicle engine control units using the OBD-II protocol.


## STATUS: Experimental - Do NOT use if your life depends on it.

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
  
### 3.1 Screenshots

!["GUI"](https://github.com/dchad/OBD-Monitor/blob/master/images/obd-gui-ss4.png "GUI Prototype")


!["GUI"](https://github.com/dchad/OBD-Monitor/blob/master/images/obd-gui-ss3.png "GUI Prototype")


!["GUI"](https://github.com/dchad/OBD-Monitor/blob/master/images/obd-gui-ss2.png "GUI Prototype")


!["GUI"](https://github.com/dchad/OBD-Monitor/blob/master/images/obd-gui-ss1.png "GUI Prototype")


## 4. On-Board Diagnostics

### 4.1 OBD Standards

   1 - OBD-II as defined by the CARB
   
   2 - OBD as defined by the EPA
   
   3 - OBD and OBD-II
   
   4 - OBD-I
   
   5 - Not OBD compliant
   
   6 - EOBD (Europe)
   
   7 - EOBD and OBD-II
   
   8 - EOBD and OBD
   
   9 - EOBD, OBD and OBD II
   
   10 - JOBD (Japan)
   
   11 - JOBD and OBD II
   
   12 - JOBD and EOBD
   
   13 - JOBD, EOBD, and OBD II
   
   14 - Reserved
   
   15 - Reserved
   
   16 - Reserved
   
   17 - Engine Manufacturer Diagnostics (EMD)
   
   18 - Engine Manufacturer Diagnostics Enhanced (EMD+)
   
   19 - Heavy Duty On-Board Diagnostics (Child/Partial) (HD OBD-C)
   
   20 - Heavy Duty On-Board Diagnostics (HD OBD)
   
   21 - World Wide Harmonized OBD (WWH OBD)
   
   22 - Reserved
 
   23 - Heavy Duty Euro OBD Stage I without NOx control (HD EOBD-I)
   
   24 - Heavy Duty Euro OBD Stage I with NOx control (HD EOBD-I N)
   
   25 - Heavy Duty Euro OBD Stage II without NOx control (HD EOBD-II)
   
   26 - Heavy Duty Euro OBD Stage II with NOx control (HD EOBD-II N)
   
   27 - Reserved
   
   28 - Brazil OBD Phase 1 (OBDBr-1)
   
   29 - Brazil OBD Phase 2 (OBDBr-2)
   
   30 - Korean OBD (KOBD)
   
   31 - India OBD I (IOBD I)
   
   32 - India OBD II (IOBD II)
   
   33 - Heavy Duty Euro OBD Stage VI (HD EOBD-IV)
   
   34-250 - Reserved
   
   251-255 - Not available for assignment (SAE J1939 special meaning)
   

### 4.2 OSI and SAE Protocols

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
 
 Note: From 2008 all vehicles must support Controller Area Network (CAN-Bus) protocols.
 
 
### 4.3 Diagnostic Test Modes

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


### 4.4 Diagnostic Trouble Codes (DTC)

  TODO:

### 4.5 Parameter Identifiers (PID)

  Parameter identifiers represent values maintained by the ECU, most are sensor
  values obtained by monitoring the numerous drive train, chassis and body
  sensors on the vehicle. The PIDs are hexadecimal values encoded as ASCII 
  characters for communication between ELM327 based OBD interpreters and the 
  laptop/tablet/smartphone client device.
  
#### 4.5.1 Selected ECU Mode 01 Parameters: 
   
   [PID] [Data Bytes] [Min Value] [Max Value] [Formula]           [Description]
   
    01    4                                    Bit Encoded        (Monitor status since DTCs cleared)
    
    04    1            0           100         100 / 255 * A      (Calculated Engine Load)
   
    05    1            -40         215         A - 40             (ECT Centigrade)
    
    0A    1            0           765         3 * A              (Fuel Pressure kPa)
    
    0B    1            0           255         A                  (MAP Pressure kPa)
    
    0C    2            0           16,383.75   (256 * A + B) / 4  (Engine RPM)
    
    0D    1            0           255         A                  (Vehicle Speed) 
    
    0E    1            -64         63.5        (A / 2) - 64       (Timing Advance: degrees before TDC)
    
    0F    1            -40         215         A - 40             (IAT Centigrade)
    
    11    1            0           100         100 / 255 * A      (Throttle Position %)
    
    2F    1            0           100         100 / 255 * A      (Fuel Tank Level %)
    
    45    1            0           100         100 / 255 * A      (Relative Throttle Position %)
    
    5A    1            0           100         100 / 255 * A      (Relative Accelerator Pedal Position %)
    
    5C    1            -40         215         A - 40             (Oil Temperature)
    
    5E    2            0           3276.75     (256 * A + B) / 20 (Fuel Flow Rate L/h)
    
    (Oil Pressure - manufacturer proprietary codes.) (Mode 22 PID 115C - GM)
    
    


#### 4.5.2 Selected ECU Mode 09 Parameters:
 
   [PID] [Data Bytes] [Description] 
   
    02    17           VIN - Vehicle Identification Number
    
    0A    20           ECU Name
    
 
## 5. Hardware Interfaces
 
### 5.1 Integrated Circuits and Projects
 
  1. ELM327 OBD Interpreter - https://www.elmelectronics.com/wp-content/uploads/2016/07/ELM327DS.pdf
  2. STN1110 OBD Interpreter - http://www.obdsol.com/solutions/chips/stn1110/
  3. Sparkfun Car Diagnostics Kit - https://www.sparkfun.com/products/10769
  4. Altronics OBD Interpreter Kit - http://www.altronics.com.au/p/k4065-obdii-automotive-interpreter-kit/
  5. Silicon Chip Magazine Feb 2010 Issue: https://www.siliconchip.com.au/

### 5.2 Pics

   TODO:

## 6. Notes

### 6.1 Troubleshooting USB-RS232 Converter Modules on Linux

    Use the following procedure if problems occur with USB-RS322
    interfaces such as the FTDI232 module on Linux:
    
    1. cd OBD-Monitor/src
    
       make stests
       
    2. Connect a wire between the USB-RS232 converter module transmit and
       receive pins, then insert the module into a USB port. 
                   
    3. Check the kernel module is loaded, for example:
                
       dmesg | grep "FTDI"
                   
    4. Check for user read/write permission on the device.
                   
       ls -la /dev/ttyUSB0
       
       sudo chmod a+rw /dev/ttyUSB0
                   
       or
                   
       chmod +s serial_test
                   
    5. Run the serial loopback test:
                
       ./serial_test
                   
    6. If serial communications still not functioning then the
       converter module may be faulty. Try swapping out the module,
       Silicon Chip Magazine sell them for $5.00AUD plus postage.
       
       
### 6.2 Troubleshooting OBD Interface (RS232 comms) In Vehicle

   If serial interface confirmed working, but no communication with
   the OBD interpreter module:
   
   1. Turn vehicle ignition OFF.
   
   2. Connect the interpreter module to vehicle OBD socket.
   
   3. Connect serial cable from the OBD interpreter module to laptop.
   
   4. Turn vehicle ignition ON, but do not start vehicle.
   
   5. Ensure interpreter module power LED indicators go ON.
   
   6. On laptop:
      
      cd OBD-Monitor/src
      
      make server
      
      make ftests
      
      chmod +s obd_monitor_server
      
      ./server_test
      
   7. Output from server_test should indicate correct OBD protocol for
      the vehicle.
      
   8. If "NO DATA" returned then the wrong OBD protocol has probably
      been selected by the interpreter auto search function.
      
   9. Set the correct OBD protocol manually with:
   
      ./server_test -p protocol-number
      
      Where protocol number is between 1 and C hexadecimal (see Section 4.2).
    
   
                   
