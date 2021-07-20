#pragma once

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MyLinked_List.h"

List getDirectories(char *input_dir);
int numOfFiles(char *dir);