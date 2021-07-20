#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int count(char * s, char c);
char *trim(char *str);
char *getLine( char * const b , size_t bsz );
size_t fget_lines(char *filename);