#pragma once 

#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include "VaccinMonitor_Utilities.h"

#define SETUP (true)


int find_occurences(char *str, char toSearch);
// Read a directory and return stats associated with it (DIR */ country / time of last check).
// Send a report for each file to the parent.
void read_directory(int opcode, char *country, char *input_dir, int write_fd, int bufferSize,List files_read);

void get_report(char *f_path);

// Check for updated files in the directories assigned to the worker.
// Send a report for each file found to the parent.
void checkForNewFiles_inCountry(int pwrite_fd, char *pinput_dir, int bufferSize,List files_read,char *country_name);

void send_report(int opcode, char *report, int write_fd, int bufferSize,int bblf, bloom_filter *bf);