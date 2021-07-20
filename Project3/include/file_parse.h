#pragma once

#include <stdio.h>  // FILE
#include "CommonStructures.h"
#include "DB_Hashes.h"
#include "VaccinMonitor_Utilities.h"

int id;

//parse file is exacti the same with the struct C_Bucket *CreateDB() function from the 1st project.
//Read file given as parameter line by line and initialises structures of monitor
void parse_file(FILE *fp);
