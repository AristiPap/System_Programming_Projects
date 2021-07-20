#include "../../include/VaccinMonitor_Utilities.h"

struct monitor_info m_structs;

//structures that are used only by monitor
void init_monitor_structs()
{
  m_structs.countries = List_create();
  m_structs.viruses = List_create();
  m_structs.citizen_ht = CHTCreate(0);
  m_structs.bblf = 0;
}

bool checkintegrity(citizen_Record *cit_ht, citizen_Record *cit,char *cname) {
    if (cit_ht->age == cit->age && strcmp(cit_ht->name, cit->name) == 0 && strcmp(cit->surname, cit->surname) == 0 && strcmp(cit_ht->country->key, cname) == 0)
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
    virus->bf = Create(6, _bf_size);
    virus->vaccinated = skiplist_init(s_height);
    virus->non_vaccinated = skiplist_init(s_height);
    virus->countries_virus = List_create();
    return virus;
}

static void CitizenDestructor(citizen_Record *person){
    free(person->ID);
    free(person->name);
    free(person->surname);
    free(person);
}

struct Bucket *CreateBucket(char *country,int id){
   struct Bucket *bu=malloc(sizeof(*bu));
   strcpy(bu->key,country);
   bu->id_country=id;
   bu->citizens=List_create();
   return bu;
}

void InsertToFilter(ListNode found, char *country, citizen_Record *citizen, char *action, char *date, size_t datasize) {
    
    if(ListFind((*(filters *)found->info).countries_virus,country,2)==NULL){    
      ListPushBack(country,(*(filters *)found->info).countries_virus,(strlen(country)+1)*sizeof(char));
    }
    if (strcmp(action, "YES") == 0) {
        Set((*(filters *)found->info).bf, citizen->ID);
        skiplist_insert((*(filters *)found->info).vaccinated, atoi(citizen->ID),citizen, date, datasize);
    } else {
        skiplist_insert((*(filters *)found->info).non_vaccinated,atoi(citizen->ID), citizen, NULL, datasize);
    }
}

//helper function: 1)checks for the integrity of the record to be inserted.
bool insertCitizenRecord(char *citizenID, char *firstName,char *lastName, char *country, int age,char *virusName, char *action, char *date,int bf_size, int s_height,int flg) {
  filters *virus;
  citizen_Record *person; 
  person = initialiser(citizenID,firstName,lastName,age);
  int id = 0;
  m_structs.citizen_ht = CHTInsert(person, m_structs.citizen_ht);
  if (h_found == true) {
    if (checkintegrity(m_structs.citizen_ht[_index].value, person, country) == false) {
        //fprintf(fd2,"ERROR IN RECORD: %s, %s, %s, %s, %d\n",person->ID,person->name,person->surname,country,person->age);
        CitizenDestructor(person);
        return false;
    }
    ListNode found = ListFind(m_structs.viruses, virusName, 1);
    // virus doesnt exist
    if (found == NULL){
        virus = VirusInitialiser(virusName,bf_size,s_height);
        ListPushBack(virus,m_structs.viruses, sizeof(*virus));
        found = ListGetLast(m_structs.viruses); 
        free(virus);
    }
    snode *vac_found;
    snode *nonvac_found;
    vac_found = skiplist_search((*(filters *)found->info).vaccinated,atoi(m_structs.citizen_ht[_index].key));
    nonvac_found = skiplist_search((*(filters *)found->info).non_vaccinated,atoi(m_structs.citizen_ht[_index].key));
    if (vac_found != NULL || nonvac_found != NULL) {
        //for flg == 0 we create the db so we dont want to vaccinate anyone that is in the non vaccinated list.
        // For flg == 1 we are using the insertCitizenRecord and vaccinateNow commands so we want to vaccinate anyone that is in the non-vaccinated list  
        CitizenDestructor(person);
        return false;
    }
    InsertToFilter(found,country,m_structs.citizen_ht[_index].value, action, date,sizeof(*(m_structs.citizen_ht[_index].value)));
    CitizenDestructor(person);
    return true;
  }  
  ListNode found1 = ListFind(m_structs.countries, country, 0);
  if (found1 == NULL) {
    struct Bucket *bu = CreateBucket(country, id);
    // create new node in list of countries and initialise the list of citizens
    ListPushBack(bu, m_structs.countries, sizeof(*bu));
    found1 = ListGetLast(m_structs.countries);
    //citizen gets updated with the info of his country of origin
    m_structs.citizen_ht = UpdateCountryPtr(m_structs.citizen_ht, _index, found1->info);
    // insert citizen
    ListPushBack(m_structs.citizen_ht[_index].value,(*(struct Bucket *)m_structs.countries->Last->info).citizens,sizeof(*m_structs.citizen_ht[_index].value));
    //free(bu);
  }
  //citizen gets updated with the info of his country of origin
  m_structs.citizen_ht = UpdateCountryPtr(m_structs.citizen_ht, _index, found1->info);
  ListPushBack(m_structs.citizen_ht[_index].value, (*(struct Bucket *)found1->info).citizens,sizeof(*m_structs.citizen_ht[_index].value));
  //found = ListGetLast((*(struct Bucket *)found->info).citizens);
    
  ListNode found = ListFind(m_structs.viruses, virusName, 1);
  // virus doesnt exist
  if (found == NULL) {
    strcat(virusName,"\0");
    virus = VirusInitialiser(virusName,bf_size,s_height);
    ListPushBack(virus, m_structs.viruses, sizeof(*virus));
    found = ListGetLast(m_structs.viruses);
    //free(virus);
  }
  strcat(country,"\0");
  strcat(action,"\0");
  strcat(date,"\0");

  InsertToFilter(found, country, m_structs.citizen_ht[_index].value, action, date,sizeof(*m_structs.citizen_ht[_index].value));
  return true; 
}

char *convert_multiple_to_message(char *virus_name,bloom_filter *bf){
    ListNode found = ListFind(m_structs.viruses,virus_name,1);
    if(found == NULL)
      return NULL;
    
    char *res = malloc((strlen(virus_name)+2)*sizeof(char));
    //char *res = malloc((strlen(virus_name) + List_size((*(filters *)found->info).countries_virus)*25 + List_size((*(filters *)found->info).countries_virus)*2+ bf->bloom_size + 1)*sizeof(char));// + ht_size(stats_ht) * 42 + 64) * sizeof(char));
    sprintf(res, "%s/", virus_name);
    int total = strlen(virus_name) + 1;  // Keep track of the next char in <res>

    struct bucket_entry *entry;
    //copy name of countries 
    for (ListNode current = (*(filters *)found->info).countries_virus->dummy->Next; current != NULL; current = current->Next){
      char buf[128];
      if(current->Next!=NULL)
        snprintf(buf,128, "%s:", current->info);
      else
        snprintf(buf,128, "%s=", current->info);
      char *tmp = malloc(total*sizeof(char));
      memcpy(tmp,res,total*sizeof(char));
      res = malloc((total+strlen(buf))*sizeof(char));
      memcpy(res,tmp,total*sizeof(char));
      //res = realloc(res,total+strlen(buf)+1);
      memcpy(res+total,buf,strlen(buf)*sizeof(char));
      //strcpy(res+total, buf);
      total += strlen(buf);
      free(tmp);
    }
    char *tmp = malloc(total*sizeof(char));
    memcpy(tmp,res,total*sizeof(char));
    res = malloc((total+ceil(bf->bloom_size / 8)+1)*sizeof(char));
    
    char *_buf = calloc(ceil(bf->bloom_size / 8),sizeof(char));
    memcpy(_buf,bf->bit_array,(ceil(bf->bloom_size / 8))*sizeof(uint8_t));
   
    
    memcpy(res,tmp,total*sizeof(char));
    memcpy(&(res[total]),_buf,ceil(bf->bloom_size / 8)*sizeof(uint8_t));
    free(tmp);
    total += ceil(bf->bloom_size / 8);
    res[total]= '\0';
    m_structs.bblf= total;
    
    return res;
}

//for each virus node we call the convert_multiple_to_message() to send a message to the parent process to read new bloom filters
char *get_blooms(int opcode, int write_fd, int bufferSize)
{
    for (ListNode current = m_structs.viruses->dummy->Next; current != NULL; current = current->Next){
      char *res = convert_multiple_to_message((*(filters *)current->info).virus_key, (*(filters *)current->info).bf);
      send_report(opcode,res,write_fd,bufferSize,m_structs.bblf,(*(filters *)current->info).bf);
    }
}

//this function has the same functionality as the above function but its used when user has added a new file 
char *updated_blooms(int opcode,pid_t ms,int write_fd, int bufferSize,char *country){
  for (ListNode current = m_structs.viruses->dummy->Next; current != NULL; current = current->Next){
    List countries = (*(filters *)current->info).countries_virus;
    ListNode found = ListFind(countries,country,2);
    if(found==NULL)
      continue;
    else{  
      char *res = convert_multiple_to_message((*(filters *)current->info).virus_key, (*(filters *)current->info).bf);
      send_report(opcode,res,write_fd,bufferSize,m_structs.bblf,(*(filters *)current->info).bf);
    }
  }
}

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
            ListDelete((*(filters *)node->info).countries_virus);
            free(node->info);
            free(node);
            node = temp;
            list->size--;
        }
    }
    free(list);
}