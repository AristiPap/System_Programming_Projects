#Aristi Papastavrou
MAKE-DIR = $(PWD)

SRC := $(MAKE-DIR)/src
INCLUDE := $(MAKE-DIR)/include

#store executables in the appropriate folders
BLD = ./obj
STRUCTS_EXE = $(BLD)/structures
TRAVELMONITOR_EXE = $(BLD)/travelMonitor
MONITORS_EXE = $(BLD)/monitor

#keep directories here
TRAVELMONITOR = $(SRC)/travelMonitor
MONITOR= $(SRC)/monitors
STRUCTURES = $(SRC)/structures
UTILS = $(SRC)/utilities

CFLAGS := -g3 -I $(INCLUDE)
LIBS := -lm
CC := gcc

# Executable file names
TRAVELMONITOR_PRG= ./travelMonitor
MONITOR_PRG = ./Monitor

COMMON_OBJS = $(STRUCTURES)/bloom.o $(STRUCTURES)/Skip_Lists.o $(STRUCTURES)/hashes.o $(STRUCTURES)/MyLinked_List.o $(STRUCTURES)/CitizenHash.o $(UTILS)/ipc.o $(UTILS)/fifos.o $(UTILS)/CommandEditing.o

MONITORS_O =  $(MONITOR)/monitors.o $(MONITOR)/signal_handling.o $(MONITOR)/dir_queries.o $(MONITOR)/file_parse.o $(MONITOR)/monitor_queries.o $(MONITOR)/VaccinMonitor_Utilities.o

TRAVEL_MONITOR_O = $(TRAVELMONITOR)/travelMonitor.o $(TRAVELMONITOR)/init_monitors.o $(TRAVELMONITOR)/tm_queries.o $(TRAVELMONITOR)/File_Utils.o $(TRAVELMONITOR)/signal_handler.o

SCARGS = $(NUM_FILES_PER_DIR)

SCRIPT = create_infiles.sh

arg1 = input_dir/inputFile.txt

arg2 = input_dir2

# Build executables
all: $(TRAVELMONITOR_PRG) $(MONITOR_PRG)
	mkdir -p $(TRAVELMONITOR_EXE)
	mkdir -p $(MONITORS_EXE)
	mkdir -p $(STRUCTS_EXE)
	mv -f $(TRAVEL_MONITOR_O) $(TRAVELMONITOR_EXE)
	mv -f $(MONITORS_O) $(MONITORS_EXE)
	mv -f $(COMMON_OBJS) $(STRUCTS_EXE)

$(TRAVELMONITOR_PRG): $(TRAVEL_MONITOR_O) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(TRAVEL_MONITOR_O) $(COMMON_OBJS) -o $(TRAVELMONITOR_PRG)

$(MONITOR_PRG): $(MONITORS_O) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(MONITORS_O) $(COMMON_OBJS) -o $(MONITOR_PRG)

script: $(TRAVELMONITOR_PRG) $(MONITOR_PRG)
	bash $(SCRIPT) $(arg1) $(arg2) $(SCARGS)

# Delete executable & object files
clean:
	rm -f $(TRAVELMONITOR_PRG)
	rm -f $(MONITOR_PRG)
	rm -rf $(BLD)
