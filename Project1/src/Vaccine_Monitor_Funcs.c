#include "../includes/Vaccine_Monitor_Funcs.h"

//helper function to check wheter a record is valid or not
static bool checkintegrity(citizen_Record *cit_ht, citizen_Record *cit,char *cname) {
    if (cit_ht->age == cit->age && strcmp(cit_ht->name, cit->name) == 0 &&
        strcmp(cit->surname, cit->surname) == 0 &&
        strcmp(cit_ht->country->key, cname) == 0)
        return true;
    else
        return false;
}

//create struct of citizen, initialised with his info (ID,name,lastname and age).Country will be initialised in a different function
citizen_Record *initialiser(char *citizenID, char *firstName, char* lastName, int age) {
    citizen_Record *person = malloc(sizeof(*person));
    person->ID = malloc(strlen(citizenID)*sizeof(char *));
    person->name = malloc(strlen(firstName)*sizeof(char *));
    person->surname = malloc(strlen(lastName)*sizeof(char *));
    strcpy(person->ID, citizenID);
    strcpy(person->name, firstName);
    strcpy(person->surname, lastName);
    person->age = age;
    return person;
}

// each node of the virus list will consist of 1 bloom filter and 2 skip lists
// for each virus. Helper function will initialise the bloom filter and skip lists of each node.
filters *VirusInitialiser(char *name,int bf_size, int s_height) {
    filters *virus = malloc(sizeof(*virus));
    strcpy(virus->virus_key, name);
    virus->bf = Create(3, bf_size);
    virus->vaccinated = skiplist_init(s_height);
    virus->non_vaccinated = skiplist_init(s_height);
    return virus;
}

//Function reads records from input file and initialises our database of citizens,countries and viruses.
struct C_Bucket *CreateDB(FILE **_stream_, List countries, List viruses,struct C_Bucket *ht, FILE *fd2,int bf_size, int s_height) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    citizen_Record *person;
    filters *virus;
    id = -1;  // id of country
    //while loop reads a record from the input file and saves in each row of the 2-d dimensional array each word from the file
    while ((read = getline(&line, &len, *_stream_)) != -1) {
        char newString[8][50];
        int j, ctr;
        j = 0;
        ctr = 0;
        for (int i = 0; i <= (strlen(line)); i++) {
            // if space or NULL found, assign NULL into newString[ctr]
            if (line[i] == ' ' || line[i] == '\0') {
                newString[ctr][j] = '\0';
                ctr++;  // for next word
                j = 0;  // index=0 for next word
            } else {
                newString[ctr][j] = line[i];
                j++;
            }
        }
        if(strcmp(newString[6],"NO")==0 && newString[7]!=NULL){
            fprintf(fd2,"ERROR IN RECORD: %s, %s, %s, %s, %d\n",newString[0],newString[1],newString[2],newString[3],atoi(newString[4]));
            continue;
        }
        
        if(strcmp(newString[6],"NO")==0)
            ht=insertCitizenRecord(newString[0], newString[1],newString[2],newString[3], atoi(newString[4]),newString[5],newString[6],"None",ht, countries,viruses,bf_size,s_height,fd2,0);  
        else
            ht=insertCitizenRecord(newString[0], newString[1],newString[2],newString[3], atoi(newString[4]),newString[5],newString[6], newString[7],ht, countries,viruses,bf_size,s_height,fd2,0);
    }
    if (line) 
        free(line);
    return ht;
}

//helper function to insert pointer to citizen record in the correct filters
void InsertToFilter(ListNode found, citizen_Record *citizen, char *action, char *date, size_t datasize) {
    if (strcmp(action, "YES") == 0) {
        Set((*(filters *)found->info).bf, citizen->ID);
        skiplist_insert((*(filters *)found->info).vaccinated, atoi(citizen->ID),citizen, date, datasize);
    } else {
        skiplist_insert((*(filters *)found->info).non_vaccinated,atoi(citizen->ID), citizen, NULL, datasize);
    }
}

//helper function: 1)checks for the integrity of the record to be inserted.
struct C_Bucket *insertCitizenRecord(char *citizenID, char *firstName,char *lastName, char *country, int age,char *virusName, char *action, char *date,struct C_Bucket *ht, List countries,List viruses,int bf_size, int s_height,FILE *fd2, int flg) {
    filters *virus;
    citizen_Record *person; 
    person = initialiser(citizenID,firstName,lastName,age);
    
    ht = CHTInsert(person, ht);
    if (h_found == true) {
        if (checkintegrity(ht[_index].value, person, country) == false) {
            fprintf(fd2,"ERROR IN RECORD: %s, %s, %s, %s, %d\n",person->ID,person->name,person->surname,country,person->age);
            CitizenDestructor(person);
            return ht;
        }
        ListNode found = ListFind(viruses, virusName, 1);
        // virus doesnt exist
        if (found == NULL) {
            virus = VirusInitialiser(virusName,bf_size,s_height);
            ListPushBack(virus, viruses, sizeof(*virus));
            found = ListGetLast(viruses);
            free(virus);
        }
        snode *vac_found;
        snode *nonvac_found;
        vac_found = skiplist_search((*(filters *)found->info).vaccinated,atoi(ht[_index].key));
        nonvac_found = skiplist_search((*(filters *)found->info).non_vaccinated,atoi(ht[_index].key));
        if (vac_found != NULL || nonvac_found != NULL) {
            fprintf(fd2,"ERROR IN RECORD: %s, %s, %s, %s, %d\n",person->ID,person->name,person->surname,country,person->age);
            if (vac_found != NULL && flg==1){
                printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %s\n",ht[_index].key, (char *)vac_found->date);
            }
            //for flg == 0 we create the db so we dont want to vaccinate anyone that is in the non vaccinated list.
            // For flg == 1 we are using the insertCitizenRecord and vaccinateNow commands so we want to vaccinate anyone that is in the non-vaccinated list  
            if(nonvac_found != NULL && flg==1){
                skiplist_delete((*(filters *)found->info).non_vaccinated,nonvac_found->key);
                skiplist_insert((*(filters *)found->info).vaccinated, atoi(ht[_index].key),ht[_index].value,date,sizeof(*ht[_index].value));
            }
            CitizenDestructor(person);
            return ht;
        }
        //if pointer of citizen doesnt exist in the skiplists we insert him to the adecuate filter
        InsertToFilter(found, ht[_index].value, action, date,sizeof(*(ht[_index].value)));
        CitizenDestructor(person);
        return ht;
    }
    ListNode found = ListFind(countries, country, 0);
    // country doesn't exist
    if (found == NULL) {
        id++;
        struct Bucket *bu = CreateBucket(country, id);
        // create new node in list of countries and initialise the list of citizens
        ListPushBack(bu, countries, sizeof(*bu));
        found = ListGetLast(countries);
        //citizen gets updated with the info of his country of origin
        ht = UpdateCountryPtr(ht, _index, found->info);
        // insert citizen
        ListPushBack(ht[_index].value,(*(struct Bucket *)countries->Last->info).citizens,sizeof(*ht[_index].value));
        free(bu);
    }
    // country exists -> insert citizen
    else {
        //citizen gets updated with the info of his country of origin
        ht = UpdateCountryPtr(ht, _index, found->info);
        ListPushBack(ht[_index].value, (*(struct Bucket *)found->info).citizens,sizeof(*ht[_index].value));
        found = ListGetLast((*(struct Bucket *)found->info).citizens);
    }
    found = ListFind(viruses, virusName, 1);
    // virus doesnt exist
    if (found == NULL) {
        virus = VirusInitialiser(virusName,bf_size,s_height);
        ListPushBack(virus, viruses, sizeof(*virus));
        found = ListGetLast(viruses);
        free(virus);
    }
    InsertToFilter(found, ht[_index].value, action, date,sizeof(*ht[_index].value));
    return ht;
}

//helper function to get the current date for vaccinateNow function
char *getCurrentDate() {
    time_t now;
    time(&now);
    char day[3], month[3];
    char year[5];
    struct tm *local = localtime(&now);
    sprintf(day, "%d", local->tm_mday);       // get day of month (1 to 31)
    sprintf(month, "%d", local->tm_mon + 1);  // get month of year (0 to 11)
    sprintf(year, "%d", local->tm_year + 1900);
    char *date = malloc(11 * sizeof(*date));

    if (strlen(day) == 1) {
        strcpy(date, "0");
        strcat(date, day);
    } else
        strcpy(date, day);
    strcat(date, "-");
    if (strlen(day) == 1) {
        strcat(date, "0");
        strcat(date, month);
    } else
        strcat(date, month);

    strcat(date, "-");
    strcat(date, year);
    return date;
}

struct C_Bucket *vaccinateNow(char *citizenID, char *firstName, char *lastName,char *country, int age, char *virusName,struct C_Bucket *ht, List countries,List viruses,int bf_size,int s_height,FILE *fd2) {
    char *date;
    date = getCurrentDate();
    ht = insertCitizenRecord(citizenID, firstName, lastName, country, age,virusName, "YES", date, ht, countries, viruses,bf_size,s_height,fd2,1);
    free(date);
    return ht;
}

//deallocate memory for the citizen record in case it doesnt get inserted
void CitizenDestructor(citizen_Record *person){
    free(person->ID);
    free(person->name);
    free(person->surname);
    free(person);
}

void DB_Destructor(struct C_Bucket *ht, List countries, List viruses) {
    //free the virus list
    VirusDestructor(viruses);
    //free the hashtable
    CHTDelete(ht, 0);
    //free list of countries
    ListofListsDelete(countries);
}

//fre the list of viruses:for each node delete the bloom filter and 2 skiplists
void VirusDestructor(List list) {
    ListNode node = list->dummy;
    ListNode temp = NULL;
    while (node != NULL) {
        temp = node->Next;
        if (node == list->dummy) {
            free(node);
            node = temp;
            list->size--;
            continue;
        } else {
            Destructor((*(filters *)node->info).bf);
            skiplist_destroy((*(filters *)node->info).vaccinated);
            skiplist_destroy((*(filters *)node->info).non_vaccinated);
            free(node->info);
            free(node);
            node = temp;
            list->size--;
        }
    }
    free(list);
}