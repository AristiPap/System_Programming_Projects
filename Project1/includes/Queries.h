#include "CommonStructures.h"
#include "MyLinked_List.h"
#include "hashes.h"

char *Uppercase(char *str);
void vaccineStatusBloom(struct bloom_filter *bf, char *citizen_id);
void vaccineStatus(skiplist *s_list, char *citizen_id);
void Percentage_Virus_Country(skiplist *vaccinated, skiplist *non_vaccinated, char *country_name,char *date1, char *date2);
int * skiplist_allcountries(skiplist *list,  List countries, int age_flg, char *date1, char *date2);
void populationStatus(List viruses,List countries, char *virus_name, char *country_name,char *date1, char *date2);
void vaccineStatusAllViruses(List viruses, char *citizen_id);
void popStatusByAge(List viruses,List countries, char *virus_name, char *country_name, char *date1, char *date2);
void Print(int *vaccines, int *non_vaccines, char *countryname);
int *Percentage_Virus_Country_age_grp(skiplist *list, char *country_name, char *date1, char *date2);
void list_nonVaccinated_Persons(List viruses, char *virusName);
int skiplist_dump(skiplist *list, char *country_name, char *date1, char *date2);
bool compare_dates(char * date1, char *date2, char *date3);