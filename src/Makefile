# Compiler flags

CC=gcc
CFLAGS=-Wall -ansi

# Linker flags

LDFLAGS=-static
LIBS=-lm
LIBDIRS=-L../../libs

# Sources

GUI_SOURCES=obd_monitor_gui.c protocols.c log.c util.c
SERVER_SOURCES=obd_monitor_server.c rs232.c log.c util.c

# Objects

GUI_EXECUTABLE=obd_gui
SERVER_EXECUTABLE=obd_server

# Includes

INCPREFIX=../../libs/
INCLUDES=-I$(INCPREFIX)/include

# Target Rules

all: $(GUI_SOURCES) $(SERVER_SOURCES)

gui: obd_monitor_gui.c obd_monitor.h protocols.h
	$(CC) $(GUI_SOURCES) $(LIBS) `pkg-config --libs --cflags gtk+-3.0` -o $(GUI_EXECUTABLE)

server: obd_monitor_server.c obd_monitor.h
	$(CC) $(CFLAGS) $(SERVER_SOURCES) -o $(SERVER_EXECUTABLE)

strip:
	strip $(SERVER_EXECUTABLE) $(GUI_EXECUTABLE)

clean:
	rm *.o $(SERVER_EXECUTABLE) $(GUI_EXECUTABLE)
