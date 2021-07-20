#pragma once

#include <stdbool.h>
#include "File_Utils.h"
#include "init_monitors.h"

#define SETUP true
#define OPERATE false

//Destructors
void TM_Destructor(struct travel_monitors *tmonitor_info, int *monitor_size,DIR *input_dir,bloom_filter *bf,List B_filters,List stats);
void DB_Destructor(List B_filters,List T_Stats);
void VirusDestructor(List list);
void ListofVirusNodes(List list);
void ListofMaster_Virus(List list);
void ListofTStats(List list);
void ListofVirusTStats(List list);
//create log for each monitor
void CreateMonitorlogs(struct travel_monitors *tmonitor_info,int monitor_size,List stats);
//althoygh implemented didnt use it as it hasnt been thoroughly debugged
pid_t revive_monitor(struct travel_monitors *tmonitor_info,int monitor_size,int index,int bufferSize,char *input_dir,int hashes,int filter_size,int height);

void signal_handler_init(void);  // Configure singal handlers & masks.
void signals_check(bloom_filter *bf,List B_filters,List stats);  // Check for pending signals  
void signals_block(void);  
void signals_unblock(void);  