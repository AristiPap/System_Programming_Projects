#include "../includes/Queries.h"
#include <ctype.h>

//calls search function of bloom filter to output the result of the query vaccinStatusBloom
void vaccineStatusBloom(struct bloom_filter *bf, char *citizen_id) {
    if (Search(bf, citizen_id) == true)
        printf("MAYBE\n");

    else
        printf("NOT VACCINATED\n");
}

//we search the vaccinated skiplist
void vaccineStatus(skiplist *s_list, char *citizen_id){

    snode *found=skiplist_search(s_list,atoi(citizen_id));
    //citizen_id wasnt found in the vaccinated skip list of the virus
    if(found==NULL)
        printf("NOT VACCINATED\n");
    else{
        printf("VACCINATED ON %s\n",(char *)found->date);
    } 
}

//for specified citizen search each vaccinated list to ckeck if he is vaccinated for all viruses in db.
void vaccineStatusAllViruses(List viruses, char *citizen_id){

    if(list_size(viruses)==0)
        return;
    for (ListNode current = viruses->dummy->Next; current != NULL; current = current->Next){
        
        snode *found=skiplist_search((*(filters *)current->info).vaccinated,atoi(citizen_id));
        if(found==NULL)
            printf("%s NO\n",(*(filters *)current->info).virus_key);
        else{
            printf("%s YES %s \n",(*(filters *)current->info).virus_key,(char *)found->date);
        }
    }

}
//loop through the specified skiplist and check if citizen apperas in this list, and if its a vaccinated skiplist ckeck if he is vaccinated between the dates given
int skiplist_dump(skiplist *list, char *country_name, char *date1, char *date2){
    snode *n = list->header;
    int ctr=0;
    while (n && n->forward[1] != list->header) {
        if(strcmp((*(citizen_Record *)n->forward[1]->value).country->key,country_name)==0){
            if(n->forward[1]->date!=NULL){
                if(compare_dates(date1,n->forward[1]->date,date2)==false){
                    n = n->forward[1];
                    continue;
                }
            }   
            ctr++;
        }    
        n = n->forward[1];
    }
    return ctr;
}
//print function to print the results of the query:populationStatus CountryName VirusName date1 date2
void Percentage_Virus_Country(skiplist *vaccinated, skiplist *non_vaccinated, char *country_name, char *date1, char *date2){

    int vac = skiplist_dump(vaccinated, country_name,date1,date2);
    int non_vac = skiplist_dump(non_vaccinated, country_name,date1,date2);
    printf("%s %d %.2f%% \n",country_name,vac, ((float)vac/(vac + non_vac))*100);
}

//same function as skiplist_dump but I create an array which will hold the result of the sum of people for each country
int * skiplist_allcountries(skiplist *list,  List countries ,int age_flg, char *date1, char *date2){

    snode *n = list->header;
    int ctr=0;
    int *counters=malloc(list_size(countries) * sizeof(int));
    
    int i=0;
    for (ListNode current = countries->dummy->Next; current != NULL; current = current->Next){
        counters[i]=0;
        i++;
    }
    
    //each value of the skip list has a pointer to the country of origin
    while (n && n->forward[1] != list->header) {
        int thesis=(*(citizen_Record *)n->forward[1]->value).country->id_country;//hash_i((*(citizen_Record *)n->forward[1]->value).country->key,1) % list_size(countries);
        if(n->forward[1]->date!=NULL){
            if(compare_dates(date1,n->forward[1]->date,date2)==false){
                n = n->forward[1];
                continue;
            }
        } 
        counters[thesis]=counters[thesis] +1;
        n = n->forward[1];
    }
    return counters;
}
//exactly the same as the previous function but each country has 4 rows to separate the sum of people according to their age group
int ** skiplist_allcountries_age_grps(skiplist *list,  List countries, char *date1, char *date2){
    /*  0-20 -> 0th row 
        20-40 -> 1st row
        40-60 -> 2nd row
        60+ -> 3rd row */
    
    snode *n = list->header;
    int ctr=0;
    int **arr = (int **)malloc(4 * sizeof(int *));
    for (int i=0; i<4; i++)
        arr[i] = (int *)malloc(list_size(countries) * sizeof(int));
    
    
    for(int i=0; i<4; i++){
        for(int j=0; j<list_size(countries); j++){
            arr[i][j]=0;            
        }
    }    
    //each value of the skip list has a pointer to the country of origin
    while (n && n->forward[1] != list->header) {
        int thesis=(*(citizen_Record *)n->forward[1]->value).country->id_country;
        if(n->forward[1]->date!=NULL){
            if(compare_dates(date1,n->forward[1]->date,date2)==false){
                n = n->forward[1];
                continue;
            }
        } 
        ctr++;
        if((*(citizen_Record *)n->forward[1]->value).age < 21){
            arr[0][thesis] = arr[0][thesis] +1;
            n = n->forward[1];
            continue;
        }
        if((*(citizen_Record *)n->forward[1]->value).age >20 && (*(citizen_Record *)n->forward[1]->value).age < 41){
            arr[1][thesis] = arr[1][thesis] +1;
            n = n->forward[1];
            continue;
        }
        if((*(citizen_Record *)n->forward[1]->value).age >40 && (*(citizen_Record *)n->forward[1]->value).age < 61){
            arr[2][thesis] = arr[2][thesis] +1;
            n = n->forward[1];
            continue;
        }
        else if((*(citizen_Record *)n->forward[1]->value).age > 60){
            arr[3][thesis] = arr[3][thesis] +1;
            n = n->forward[1];
            continue;
        }   
        n = n->forward[1];
    }
    return arr;
}

void populationStatus(List viruses,List countries, char *virus_name, char *country_name, char *date1, char *date2){
    
    int flg=0;
    ListNode virus = ListFind(viruses, virus_name, 1);
    if(virus==NULL){
        printf("ERROR: INPUT VIRUS DOESNT EXIST IN THE DATABASE\n");
        return;
    }
    //if country name specified then we use the Percentage_Virus_Country
    if(strcmp(country_name,"None")!=0){
        Percentage_Virus_Country((*(filters *)virus->info).vaccinated,(*(filters *)virus->info).non_vaccinated,country_name,date1,date2);
        return;
    }
    //else we call the skiplist_allcountries_age_grps for both skiplists where each cell represents the results of each country
    float percentage;
    int *counters = skiplist_allcountries((*(filters *)virus->info).vaccinated,countries,0,date1,date2);
    int *counters_nonvac = skiplist_allcountries((*(filters *)virus->info).non_vaccinated,countries,0,date1,date2);
    int i=0;
    for (ListNode current = countries->dummy->Next; current != NULL; current = current->Next){
        int thesis=(*(struct Bucket *)current->info).id_country;//hash_i((*(struct Bucket *)current->info).key,1) % list_size(countries);
        if(counters[thesis] + counters_nonvac[thesis] == 0 || counters[thesis]==0)
            percentage = 0;
        else{
            percentage=((float)counters[thesis]/(counters[thesis] + counters_nonvac[thesis]))*100;
        }
        printf("%s %d  %.2f%% \n",(*(struct Bucket *)current->info).key,counters[thesis],percentage);
    }
    free(counters); 
    free(counters_nonvac);
}

//has the same result and usage as  skiplist_allcountries_age_grps() but now we have a 1-d array because we only hold results for 1 specified country
int * Percentage_Virus_Country_age_grp(skiplist *list, char *country_name, char *date1, char *date2){
    
    snode *n = list->header;
    int *ctrs = malloc(4*sizeof(int));
    for(int i=0; i<4; i++)
        ctrs[i]=0;
    while (n && n->forward[1] != list->header){
        if(strcmp((*(citizen_Record *)n->forward[1]->value).country->key,country_name)==0){
            if(n->forward[1]->date!=NULL){
                if(compare_dates(date1,n->forward[1]->date,date2)==false){
                    n = n->forward[1];
                    continue;
                }
            } 
            //if(compare_dates(date1,n->forward[1]->date,date2)==true){
            if((*(citizen_Record *)n->forward[1]->value).age < 21){
                ctrs[0] = ctrs[0] + 1;
                n = n->forward[1];
                continue;
            }
            if((*(citizen_Record *)n->forward[1]->value).age > 20 && (*(citizen_Record *)n->forward[1]->value).age  < 41){
                ctrs[1] = ctrs[1] + 1;
                n = n->forward[1];
                continue;
            }
            if(40 < (*(citizen_Record *)n->forward[1]->value).age && (*(citizen_Record *)n->forward[1]->value).age < 61){
                ctrs[2] = ctrs[2] + 1;
                n = n->forward[1];
                continue;
            }
            else if((*(citizen_Record *)n->forward[1]->value).age > 60 ){
                ctrs[3] = ctrs[3] + 1;
                n = n->forward[1];
                continue;
            }
            //}    
        }
        n = n->forward[1];
    }
    return ctrs;
}

// print percentage results and check for NaN results so not to print them
void Print(int *vaccines, int *non_vaccines, char *countryname){
    printf("\n%s\n",countryname);
    float percentage;
    if(vaccines[0] + non_vaccines[0] == 0)
        percentage = 0;
    else
        percentage=((float)vaccines[0]/(vaccines[0] + non_vaccines[0]))*100;
    printf("0-20 %d %.2f%%\n", vaccines[0],percentage);
    if(vaccines[1] + non_vaccines[1] == 0)
        percentage = 0;
    else
        percentage=((float)vaccines[1]/(vaccines[1] + non_vaccines[1]))*100;
    printf("20-40 %d %.2f%%\n", vaccines[1],percentage);
    if(vaccines[2] + non_vaccines[2] == 0)
        percentage = 0;
    else
        percentage=((float)vaccines[2]/(vaccines[2] + non_vaccines[2]))*100;
    printf("40-60 %d %.2f%%\n", vaccines[2],percentage);
    if(vaccines[3] + non_vaccines[3] == 0)
        percentage = 0;
    else
        percentage=((float)vaccines[3]/(vaccines[3] + non_vaccines[3]))*100;
    printf("60+ %d %.2f%%\n", vaccines[3],percentage);
}

void popStatusByAge(List viruses,List countries, char *virus_name, char *country_name,char *date1, char *date2){
    
    int flg=0;
    ListNode virus = ListFind(viruses, virus_name, 1);
    if(virus==NULL){
        printf("ERROR: INPUT VIRUS DOESNT EXIST IN THE DATABASE\n");
        return;
    }
    //if country name specified then we use the Percentage_Virus_Country_age_grp
    if(strcmp(country_name,"None")!=0){
        int *vaccines = Percentage_Virus_Country_age_grp((*(filters *)virus->info).vaccinated,country_name,date1,date2);
        int *non_vaccines = Percentage_Virus_Country_age_grp((*(filters *)virus->info).non_vaccinated,country_name,date1,date2);
        Print(vaccines,non_vaccines,country_name);
        free(vaccines);
        free(non_vaccines);
        return;
    }
    //else we call the skiplist_allcountries_age_grps for both skiplists where each column representes the results of each country
    float percentage;
    int tmp[4];
    int tmp2[4];
    int **counters = skiplist_allcountries_age_grps((*(filters *)virus->info).vaccinated,countries,date1,date2);
    int **counters_nonvac = skiplist_allcountries_age_grps((*(filters *)virus->info).non_vaccinated,countries,date1,date2);
    for (ListNode current = countries->dummy->Next; current != NULL; current = current->Next){
        int thesis=(*(struct Bucket *)current->info).id_country;
        for(int i=0; i<4; i++){
            tmp[i]=counters[i][thesis];
            tmp2[i]=counters_nonvac[i][thesis];
        }
        Print(tmp,tmp2,(*(struct Bucket *)current->info).key);
    }
    for(int i=0; i<4; i++){
        free(counters[i]);
        free(counters_nonvac[i]);
    }
    free(counters);
    free(counters_nonvac);
}
//find the skip list of non_vaccinated citizens of the virus given and iterate the 1st level
void list_nonVaccinated_Persons(List viruses, char *virusName){
    ListNode virus = ListFind(viruses,virusName,1);
    if(virus==NULL){
        printf("ERROR: INPUT VIRUS DOESNT EXIST IN THE DATABASE\n");
        return;
    }
    snode *n = (*(filters *)virus->info).non_vaccinated->header;
    int ctr=0;
    while (n && n->forward[1] != (*(filters *)virus->info).non_vaccinated->header) {
        printf("%s %s %s %s %d \n",(*(citizen_Record *)n->forward[1]->value).ID,(*(citizen_Record *)n->forward[1]->value).name,(*(citizen_Record *)n->forward[1]->value).surname,(*(citizen_Record *)n->forward[1]->value).country->key,(*(citizen_Record *)n->forward[1]->value).age);
        ctr++;
        n = n->forward[1];
    }
 }
 
 //function to check if date 2 is between date1 and date3 and if date1 and date3 are in the right order
 bool compare_dates(char * date1, char *date2, char *date3){
    int day1 , day2, day3, month1, month2, month3, year1, year2, year3;
    char *date;
    char _date1[11],_date2[11],_date3[11];
      
    strcpy(_date1,date1);
    
    date = strtok(_date1,"-");
    day1 = atoi(date);
    
    date = strtok(NULL,"-");
    month1 = atoi(date);
    
    date = strtok(NULL,"-");
    year1 = atoi(date);
    
    strcpy(_date2,date2);
    date = strtok(_date2,"-");
    day2 = atoi(date);
    
    date = strtok(NULL,"-");
    month2 = atoi(date);
    
    date = strtok(NULL,"-");
    year2 = atoi(date);
    
    strcpy(_date3,date3);
    date = strtok(_date3,"-");
    day3 = atoi(date);

    date = strtok(NULL,"-");
    month3 = atoi(date);
    
    date = strtok(NULL,"-");
    year3 = atoi(date);
    
    if(year2 < year1 || year2 > year3)
        return false;
    if(year2==year1){
        if(month2<month1)
            return false;
        else if(month1==month2){
            if(day2<day1)
                return false;
        }
    }
    if(year2==year3){
        if(month2>month3)
            return false;
        else if(month1==month3){
            if(day2>day3)
                return false;
        }
    }
    return true;
}