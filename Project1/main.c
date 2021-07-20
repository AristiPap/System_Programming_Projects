#include "includes/Vaccine_Monitor_Funcs.h"
#include "includes/Queries.h"
#include "includes/Utilities.h"
#include <math.h>

static int find_command(char *_command){
    int size=strlen(_command);
    char command[size];
    strcpy(command,_command);
    if(_command == NULL || _command[0]=='\0')
        return -1;
        
    if(strcmp(command,"exit")==0)
        return 1;
        
    if(strcmp(command,"vaccineStatusBloom")==0)
        return 2;
            
    if(strcmp(command,"vaccineStatus")==0)
        return 3;
    
    if(strcmp(command,"populationStatus")==0)
        return 4;
        
    if(strcmp(command,"popStatusByAge")==0)
        return 5;
    
    if(strcmp(command,"insertCitizenRecord")==0)
        return 6;
    
    if(strcmp(command,"vaccinateNow")==0)
        return 7;
    
    if(strcmp(command,"list-nonVaccinated-Persons")==0)
        return 8;
    
    if(strcmp(command,"-h")==0)
        return 9;
        
    return -1;
}

static void help(){
    printf("The commands you can enter are the following:\n");
    printf("~ vaccineStatusBloom citizenID virusName\n~vaccineStatus citizenID virusName\n~vaccineStatus citizenID\n~populationStatus [country] virusName date1 date2\n~popStatusByAge [country] virusName date1 date2\n~insertCitizenRecord citizenID firstName lastName country age virusName YES/NO [date]\n~/vaccinateNow citizenID firstName lastName country age virusName \n~list-nonVaccinated-Persons virusName\n~exit\n");
}

//./vaccineMonitor -c citizenRecordsFile –b bloomSize
int main(int argc, char const *argv[]){
    
    if(argc!=5){
        printf("You entered wrong input.You have to type:./vaccineMonitor -c citizenRecordsFile –b bloomSize");
        return 0;
    }
    int bf_size = atoi(argv[4]);
    char *file_path =  malloc(strlen(argv[2])*sizeof(char *));
    if(strcmp(argv[2],"input_dir/inputFile.txt")==0)
        strcpy(file_path,argv[2]);
    else
        strcpy(file_path,"input_dir/inputFile.txt");
    
    FILE *fd1 = fopen(file_path,"r");
    FILE *fd2 = fopen("./errors/error_log.txt","w");
    int p=0;
    struct C_Bucket *ht = CHTCreate(0);
    List countries = list_create();
    List viruses = list_create();
    //printf("%d",fget_lines(file_path));
    int s_height = ((int)log(fget_lines(file_path)));
    ht = CreateDB(&fd1,countries,viruses,ht,fd2,bf_size,s_height);
    
    char *line=NULL;
    char cp[BUF_SIZE];
    char *command;
    size_t len = 0;
    ssize_t read;
    while(1){
        printf("/");           
        getline(&line, &len, stdin);
        strtok(line, "\n");
        trim(line);
        strcpy(cp,line);
        int size=strlen(line); 
        command = strtok(line," ");
        p=find_command(command);
        char ID[5];
        char vname[30];
        char date1[11];
        char date2[11];
        char cname[30];
        char name[13];
        char lastname[13];
        int age;
        char action[4];
        switch(p)
        {
              case 0: 
                break;
              case 1: 
                DB_Destructor(ht,countries,viruses);
                free(line);
                free(file_path);
                fclose(fd1);
                fclose(fd2);
                exit(EXIT_SUCCESS);
                break;
              case 2:
                command = strtok(NULL," ");
                strcpy(ID,command);
                command = strtok(NULL," ");
                strcpy(vname,command);
                ListNode found = ListFind(viruses,vname,1);
                vaccineStatusBloom((*(filters *)found->info).bf,ID);
                break;
              case 3:
                command = strtok(NULL," ");
                strcpy(ID,command);
                if(count(cp,' ')>1){
                    command = strtok(NULL," ");
                    strcpy(vname,command);
                    ListNode found = ListFind(viruses,vname,1);
                    vaccineStatus((*(filters *)found->info).vaccinated,ID);
                    break;
                }
                vaccineStatusAllViruses(viruses,ID);
                break;
              case 4:
                //populationStatus [country] virusName date1 date2
                if(count(cp,'-')<4)
                    break;
                if(count(cp,' ')==3){
                    command = strtok(NULL," ");
                    strcpy(vname,command);
                    command = strtok(NULL," ");
                    strcpy(date1,command);
                    command = strtok(NULL," ");
                    strcpy(date2,command);
                    populationStatus(viruses,countries, vname, "None",date1,date2);
                    break;
                }
                else{
                    command = strtok(NULL," ");
                    strcpy(cname,command);
                    command = strtok(NULL," ");
                    strcpy(vname,command);
                    command = strtok(NULL," ");
                    strcpy(date1,command);
                    command = strtok(NULL," ");
                    strcpy(date2,command);
                    populationStatus(viruses,countries, vname, cname,date1,date2);
                    break;
                }
              case 5:
                //popStatusByAge [country] virusName date1 date2
                if(count(cp,'-')<4)
                    break;
                if(count(cp,' ')==3){
                    command = strtok(NULL," ");
                    strcpy(vname,command);
                    command = strtok(NULL," ");
                    strcpy(date1,command);
                    command = strtok(NULL," ");
                    strcpy(date2,command);
                    popStatusByAge(viruses,countries, vname,"None",date1,date2);
                    break;
                }
                else{
                    command = strtok(NULL," ");
                    strcpy(cname,command);
                    command = strtok(NULL," ");
                    strcpy(vname,command);
                    command = strtok(NULL," ");
                    strcpy(date1,command);
                    command = strtok(NULL," ");
                    strcpy(date2,command);
                    popStatusByAge(viruses,countries, vname, cname,date1,date2);
                    break;
                }
              case 6:
              //insertCitizenRecord citizenID firstName lastName country age virusName YES/NO [date]
                command = strtok(NULL," ");
                strcpy(ID,command);
                command = strtok(NULL," ");
                strcpy(name,command);
                command = strtok(NULL," ");
                strcpy(lastname,command);
                command = strtok(NULL," ");
                strcpy(cname,command);
                command = strtok(NULL," ");
                age = atoi(command);
                command = strtok(NULL," ");
                strcpy(vname,command);
                command = strtok(NULL," ");
                strcpy(action,command);
                
                if(count(cp,' ')==8){
                    command = strtok(NULL," ");
                    strcpy(date1,command);
                    ht=insertCitizenRecord(ID,name,lastname,cname,age,vname,action,date1,ht,countries,viruses,bf_size,s_height,fd2,1);
                    break;
                }
                else{
                    ht=insertCitizenRecord(ID,name,lastname,cname,age,vname,action,"None",ht,countries,viruses,bf_size,s_height,fd2,1);
                    break;
                }
              case 7:
                command = strtok(NULL," ");
                strcpy(ID,command);
                command = strtok(NULL," ");
                strcpy(name,command);
                command = strtok(NULL," ");
                strcpy(lastname,command);
                command = strtok(NULL," ");
                strcpy(cname,command);
                command = strtok(NULL," ");
                age = atoi(command);
                command = strtok(NULL," ");
                strcpy(vname,command);
                ht = vaccinateNow(ID, name,lastname,cname,age,vname,ht, countries,viruses,bf_size,s_height,fd2);
                break;
              case 8:
                command = strtok(NULL," ");
                strcpy(vname,command);
                list_nonVaccinated_Persons(viruses, vname);       
                break;
              case 9:
                help();
                break;
              default:
                printf("ERROR: %d: Incorrect menu option, type -h if you want to see the menu option!\n", -1);
        }
    }
}