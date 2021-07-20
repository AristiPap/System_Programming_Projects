#!/bin/bash

RANDOM=$$  # Initialize with a random seed

###################################################################
# Argument parse & error handle
USAGE="./testFile.sh virusesFile countriesFile input_dir duplicatesAllowed records"

if (("$#" != 5)); then
  echo $'\n'"Please enter 5 arguements."
  echo "> Usage: $USAGE" $'\n'
  exit 5
fi

VIRUS_FILE=$1
if [ ! -e $VIRUS_FILE ]; then
  echo $'\n'"Arguement 1 -> Virus file \"$VIRUS_FILE\" doesn't exist."
  echo "> Usage: $USAGE" $'\n'
  exit 1
fi

COUNTRY_FILE=$2
if [ ! -e $COUNTRY_FILE ]; then
  echo $'\n'"Arguement 2 -> Country file \"$COUNTRY_FILE\" doesn't exist."
  echo "> Usage: $USAGE" $'\n'
  exit 2
fi

INPUT_DIR=$3  # No need to check; if it exists, it will be erased & replaced

DUPLICATE_FLG=$4
if (("$DUPLICATE_FLG" < 0 || "$DUPLICATE_FLG" > 1)); then
  echo $'\n'"Arguement 3 -> Please enter either 0 for no duplicates or 1 for duplicates in record file."
  echo "> Usage: $USAGE" $'\n'
  exit 3
fi

RECORDS=$5
if (("$RECORDS" <= 0)); then
  echo $'\n'"Arguement 4 -> Please enter positive number of records per file."
  echo "> Usage: $USAGE" $'\n'
  exit 4
fi

###################################################################
VACCINATED=(YES NO)
RECORD_ID=0

#creates a string that contains both name and surname. By using only once the /dev/urandom we decrease the time of execution
#In the big string created, the first ~name_length characters are characters create the name and the rest create the surname
#The separation of the name and surname happend in the create_record or create_duprec
create_name() 
{
  NAME=""
  declare -i name_length=$(shuf -i 3-12 -n 1)
  FLENGTH=$name_length
  declare -i surname_length=$(shuf -i 3-12 -n 1)
  LLENGTH=$surname_length
  NAME=$(LC_ALL=C tr -dc 'A-Za-z' </dev/urandom | head -c $(( name_length + surname_length )) ; echo)
}

#Creates random date of vaccination of citizen, between the year of hiw birth and 2021(current year)
create_date(){
	AGE=$(( RANDOM % 120 + 1 ))
	BIRTH=$(( 2021-$AGE ))
	YEAR=$(shuf -i $BIRTH-2021 -n 1)
	NOW=$(date -d "$YEAR-$((RANDOM%12+1))-$((RANDOM%28+1))" '+%d-%m-%Y')
}

#Creates unique records.We call this function when $DUPLICATE_FLG" == 1
create_duprec()
{
  create_name
  OFFSET=$FLENGTH
  FLNAME=$NAME
  SURNAMELEN=$LLENGTH
  FNAME=${FLNAME:0:$OFFSET}   
  LNAME=${FLNAME:$OFFSET}
  ACTION=${VACCINATED[($RANDOM%2)]}
  #The record of each citizen will have the following format:
  #Citizen_ID Name Surname Country Age Disease Date Action
  create_date
  DUP_L=$(shuf -i 1-$RECORD_ID -n 1)
  if(( "$1" % 5 == 0 & "$1"!= 0)); then
    TRY=`awk NR==$DUP_L inputFile.txt`
    ID=${TRY:0:1}
    vars=( $TRY )
    if [ "$ACTION" = "${VACCINATED[0]}" ]; then
      RECORD="$ID ${vars[1]} ${vars[2]} ${vars[3]} ${vars[4]} ${VIRUSES[$((RANDOM % $VIRUS_NUM ))]} $ACTION $NOW"
    else
      RECORD="$ID ${vars[1]} ${vars[2]} ${vars[3]} ${vars[4]} ${VIRUSES[$((RANDOM % $VIRUS_NUM ))]} $ACTION "
    fi
  else
    if [ "$ACTION" = "${VACCINATED[0]}" ]; then
      RECORD="$RECORD_ID $FNAME $LNAME ${COUNTRIES[$((RANDOM % $COUNTRIES_NUM ))]} $AGE ${VIRUSES[$((RANDOM % $VIRUS_NUM ))]} $ACTION $NOW "
    else
      RECORD="$RECORD_ID $FNAME $LNAME ${COUNTRIES[$((RANDOM % $COUNTRIES_NUM ))]} $AGE ${VIRUSES[$((RANDOM % $VIRUS_NUM ))]} $ACTION "
    fi
    let "RECORD_ID = RECORD_ID + 1"
  fi
}

#Creates unique records.We call this function when $DUPLICATE_FLG" == 0
create_record()
{
  create_name
  OFFSET=$FLENGTH
  FLNAME=$NAME
  SURNAMELEN=$LLENGTH
  FNAME=${FLNAME:0:$OFFSET}   
  LNAME=${FLNAME:$OFFSET}
  ACTION=${VACCINATED[($RANDOM%2)]}
  #The record of each citizen will have the following format:
  #Citizen_ID Name Surname Country Age Disease Date Action
   #$(shuf -i 0-$RECORDS -n 1)
  if [ "$ACTION" = "${VACCINATED[0]}" ]; then
    create_date
    RECORD="${REC_IDS[RECORD_ID]} $FNAME $LNAME ${COUNTRIES[$((RANDOM % $COUNTRIES_NUM ))]} $AGE ${VIRUSES[$((RANDOM % $VIRUS_NUM ))]} $ACTION $NOW "
  else
    RECORD="${REC_IDS[RECORD_ID]} $FNAME $LNAME ${COUNTRIES[$((RANDOM % $COUNTRIES_NUM ))]} $AGE ${VIRUSES[$((RANDOM % $VIRUS_NUM ))]} $ACTION "
  fi
  let "RECORD_ID = RECORD_ID + 1"
}

##################################################################

read_countries() # Reads the countries from the countriesFile.txt and stores them in array $COUNTRIES 
{                
  COUNTRIES=()
  while read line; do
    COUNTRIES+=($line)
  done < $COUNTRY_FILE
  COUNTRIES_NUM=${#COUNTRIES[@]}
}

read_viruses() # Reads the viruses from the virusesFile.txt and store them in array $VIRUSES
{               
  VIRUSES=()
  while read line; do
    VIRUSES+=($line)
  done < $VIRUS_FILE
  VIRUS_NUM=${#VIRUSES[@]}
}

create_rec_ids()
{
  REC_IDS=()
  for (( l = 0; l < RECORDS; l++ )); do
    REC_IDS+=($l)
  done
  REC_IDS=( $(shuf -e "${REC_IDS[@]}") )
}

#creates directory and the file with records for input
create_file()
{
  if [ -d $INPUT_DIR ]; then  # if DIR exists, delete it
    rm -rf $INPUT_DIR
  fi

  mkdir $INPUT_DIR  # Create input dir
  cd $INPUT_DIR
  touch inputFile.txt
  if(( "$DUPLICATE_FLG" == 1)); then
    for (( l = 0; l < RECORDS; l++ )); do  # Fill the date files with records
      create_duprec "l"
      echo $RECORD >> inputFile.txt
    done
  else
    create_rec_ids
    RECORD_ID=0
    for (( l = 0; l < RECORDS; l++ )); do  # Fill the date files with records
      create_record
      echo $RECORD >> inputFile.txt
    done
  fi
  cd ..
}
############################ Main ############################## 
read_viruses
read_countries
create_file
exit 0
################################################################