# Project: OBD Monitor
# Author:  Derek Chadwick
# Date:    01/12/2017


# Compiler flags

CC=gcc
CFLAGS=-Wall

# Linker flags

LDFLAGS=-static
LIBS=-lm
LIBDIRS=-L../../libs

# Sources

GUI_SOURCES=obd_monitor_gui.c protocols.c sockets.c gui_dialogs.c gui_gauges.c log.c util.c gui_gauges_aux.c config.c pid_hash_map.c
SERVER_SOURCES=obd_monitor_server.c rs232.c log.c util.c
SIMULATOR_SOURCES=ecu_simulator.c rs232.c log.c util.c
UNIT_TEST_SOURCES=unit_test.c util.c log.c rs232.c pid_hash_map.c dtc_hash_map.c config.c
FUNCTION_TEST_SOURCES=test_server.c sockets.c util.c log.c
SERIAL_TEST_SOURCES=test_serial_rxtx.c rs232.c

# Objects

GUI_EXECUTABLE=obd_gui
SERVER_EXECUTABLE=obd_server
SIMULATOR_EXECUTABLE=ecu_sim
UNIT_TEST_EXECUTABLE=unit_test
FUNCTION_TEST_EXECUTABLE=server_test
SERIAL_TEST_EXECUTABLE=serial_test

# Includes

INCPREFIX=../../libs/
INCLUDES=-I$(INCPREFIX)/include

# Target Rules

all: gui server simulator utests ftests stests

gui: obd_monitor_gui.c obd_monitor.h protocols.h
	$(CC) $(GUI_SOURCES) $(LIBS) `pkg-config --libs --cflags gtk+-3.0` -o $(GUI_EXECUTABLE)

server: obd_monitor_server.c obd_monitor.h
	$(CC) $(CFLAGS) $(SERVER_SOURCES) -o $(SERVER_EXECUTABLE)

simulator: ecu_simulator.c obd_monitor.h
	$(CC) $(CFLAGS) $(SIMULATOR_SOURCES) -o $(SIMULATOR_EXECUTABLE)

utests: unit_test.c obd_monitor.h
	$(CC) $(CFLAGS) $(UNIT_TEST_SOURCES) -o $(UNIT_TEST_EXECUTABLE)
	
ftests: test_server.c obd_monitor.h
	$(CC) $(CFLAGS) $(FUNCTION_TEST_SOURCES) -o $(FUNCTION_TEST_EXECUTABLE)
	
stests: test_serial_rxtx.c
	$(CC) $(CFLAGS) $(SERIAL_TEST_SOURCES) -o $(SERIAL_TEST_EXECUTABLE)
	
strip:
	strip $(SERVER_EXECUTABLE) $(GUI_EXECUTABLE)

clean:
	rm $(SERVER_EXECUTABLE) $(GUI_EXECUTABLE) $(SIMULATOR_EXECUTABLE) $(UNIT_TEST_EXECUTABLE) $(FUNCTION_TEST_EXECUTABLE) $(SERIAL_TEST_EXECUTABLE)
	
	
