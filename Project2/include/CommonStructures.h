#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "bloom.h"
#include "Skip_Lists.h"
#include <stdbool.h>
#include "MyLinked_List.h"

struct Bucket{
  List citizens;
  int id_country;
  char key[17]; //country name
}Bucket;

typedef struct filters{
  char virus_key[17];
  struct bloom_filter *bf;
  skiplist *vaccinated;
  skiplist *non_vaccinated;
  List countries_virus;
}filters; 

typedef struct citizen_Record{
  char *ID;
  char *name;//[17];
  char *surname;//[17];
  int age;
  struct Bucket *country;
}citizen_Record;

int id_country;