#pragma once
#include "bloom.h"
#include "Skip_Lists.h"
#include "MyLinked_List.h"
#include "File_Utils.h"
#include "VaccinMonitor_Utilities.h"
#include "networking.h"
struct travel_monitors
{
  pid_t w_pid;  //  PID of mnonitor
  int port;
  int read_fd;  // read fd for travel monitor to read
  int write_fd;  // write fd for travel monitor to write
  List countries; //countries that the monitor will use to initialise its structs
};

pid_t init_monitor_info(struct travel_monitors *tmonitor_info, int index, char *bufferSize_str, char *input_dir,int hashes,int filter_size,int height,uint16_t port);

//travel monitor calls this function to create as many minitors as given by the user
void init_monitors(struct travel_monitors *monitors, int monitor_size, int bufferSize,int cyclic_bufferSize,int num_threads,char *input_dir,int hashes,int filter_size,int height);

//assign countries to monitors
void assign_countries(struct travel_monitors *tmonitor_info, int monitor_size, DIR *input_dir, int bufferSize);
