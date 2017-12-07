# Compiler flags

CC=gcc
CFLAGS=-Wall -ansi

# Linker flags

LDFLAGS=-static
LIBS=
LIBDIRS=-L../../libs

# Sources

GUI_SOURCES=obd_monitor_gui.c
SERVER_SOURCES=obd_monitor_server.c rs232.c

# Objects

GUI_EXECUTABLE=obd_gui
SERVER_EXECUTABLE=obd_server

# Includes

INCPREFIX=../../libs/
INCLUDES=-I$(INCPREFIX)/include

# Target Rules

all: $(GUI_SOURCES) $(GUI_EXECUTABLE) $(SERVER_SOURCES) $(SERVER_EXECUTABLE)

gui: $(CC) $(CFLAGS) $(GUI_SOURCES) `pkg-config gtk+-3.0` -o $(GUI_EXECUTABLE)

server: $(CC) $(CFLAGS) $(SERVER_SOURCES) -o $(SERVER_EXECUTABLE)

strip:
	strip $(SERVER_EXECUTABLE) $(GUI_EXECUTABLE)

clean:
	rm *.o $(SERVER_EXECUTABLE) $(GUI_EXECUTABLE)
