#pragma once

#include "init_monitors.h"
#include "dir_queries.h"
#include "sockets.h"

#define JUST_PRINT true
#define UPDATE_DATA false


typedef struct virusNode{
    bloom_filter *blf;
    List country_names;
}virusNode;

typedef struct master_virus{
    char virus_name[25];
    List b_filters; //each virus node has a list, where each node is a bloom filter with its list of countries
}master_virus;

//master monitor will have a list with as many nodes as the viruses that we have found in the files
//each node will have a list of countries with their counters for accepted and rejected requests
typedef struct Date_Info{
    char date[11];
    bool acceptance;
}Date_Info;

typedef struct travel_stat_info{
    char *country_name;
    List date_info;
}travel_stat_info;

typedef struct virus_travel_stats{
    //List of nodes of type travel_stat_info
    char *virus_type;
    List travel_stats;
}virus_travel_stats;


int travelRequest(int *monitor_fds,struct travel_monitors *tmonitor_info, int monitor_size, int bufferSize,char *citizen_id, char *date,char *countryFrom,char *countryTo,char *virus_name, List viruses,List viruses_travel_stats);
Date_Info* date_init(char *date,bool accepted);
travel_stat_info *TStatInfo(char *country_name);
virus_travel_stats *VTravelStats(char *country_name,char *virus_name);
void travelStats(char *virusName,char *date1,char *date2,char *country,List viruses_travel_stats);
void searchVaccinationStatus(struct travel_monitors *tmonitor_info, int monitor_size, int bufferSize,char *citizenID);
void init_db(char *msg,List viruses,int hashes,int filter_size, bloom_filter *new_bf);
void update_db(char *msg,List viruses,int hashes,int filter_size, bloom_filter *new_bf);
void CreateMonitorlogs(struct travel_monitors *tmonitor_info,int monitor_size,List stats);
