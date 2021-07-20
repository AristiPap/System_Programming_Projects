#pragma once

#include "DB_Hashes.h"
#include "CommonStructures.h"
#include "dir_queries.h"
#include "header.h"


typedef struct monitor_info{
    struct C_Bucket *citizen_ht;    //Citizens hash table
    List countries; //List of countries where each country node has a list of citizens
    List viruses; //List of viruses, where each node has the virus_name,one bloom filter, 2 skip lists and a list of the country names that have info for the virus
    int bblf;
}monitor_info;

void init_monitor_structs();
bool checkintegrity(citizen_Record *cit_ht, citizen_Record *cit,char *cname);
citizen_Record *initialiser(char *citizenID, char *firstName, char* lastName, int age);
filters *VirusInitialiser(char *name,int bf_size, int s_height);
void InsertToFilter(ListNode found,char *country, citizen_Record *citizen, char *action, char *date, size_t datasize);
bool insertCitizenRecord(char *citizenID, char *firstName,char *lastName, char *country, int age,char *virusName, char *action, char *date,int bf_size, int s_height,int flg);
void insert_country(char *country);
char *get_blooms(int opcode, int write_fd, int bufferSize);
char *convert_multiple_to_message(char *virus_name,bloom_filter *bf);
char *updated_blooms(int opcode,pid_t ms,int write_fd, int bufferSize,char *country);
//int DB_Destructor();