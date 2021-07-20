#pragma once
#include "DB_Hashes.h"
#include <time.h>

#define BUF_SIZE 256
int id;

citizen_Record *initialiser(char *citizenID, char *firstName, char* lastName, int age);
filters *VirusInitialiser(char *virus,int bf_size,int s_height);
struct C_Bucket *CreateDB(FILE **_stream_,List mylist,List viruses,struct C_Bucket *ht,FILE *fd2,int bf_size,int s_height);
void InsertToFilter(ListNode found,citizen_Record * citizen, char *action,char *date, size_t datasize);
void CitizenDestructor(citizen_Record *person);
void VirusDestructor(List list);
void DB_Destructor(struct C_Bucket *ht, List countries, List viruses);
struct C_Bucket * insertCitizenRecord(char *citizenID, char * firstName, char *lastName, char *country, int age, char *virusName, char *action, char *date, struct C_Bucket *ht, List countries,List viruses,int bf_size,int s_height,FILE *fd2,int flg);
struct C_Bucket * vaccinateNow(char *citizenID, char * firstName, char *lastName, char *country, int age, char *virusName,struct C_Bucket *ht, List countries,List viruses,int bf_size,int s_height,FILE *fd2);
char * getCurrentDate();