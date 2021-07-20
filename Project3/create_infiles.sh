#!/bin/bash

RANDOM=$$  # Initialize with a random seed

###################################################################
# Argument parse & error handle
USAGE="./create_infiles.sh inputFile input_dir numFilesPerDirectory"

if (("$#" != 3)); then
  echo $'\n'"Please enter 3 arguements."
  echo "> Usage: $USAGE" $'\n'
  exit 5
fi

INPUT_FILE=$1
if [ ! -e $VIRUS_FILE ]; then
  echo $'\n'"Arguement 1 -> Input file \"$INPUT_FILE\" doesn't exist."
  echo "> Usage: $USAGE" $'\n'
  exit 1
fi

INPUT_DIR=$2  # No need to check; if it exists, it will be erased & replaced
if [ -d $INPUT_DIR ]; then  # if DIR exists, delete it
    echo $'\n'"Arguement 2 -> Input Directory  \"$INPUT_DIR\" already exists."
    echo "> Usage: $USAGE" $'\n'
    exit 2
fi

FILES_DIRECTORY=$3
if (("$FILES_DIRECTORY" <= 0)); then
  echo $'\n'"Arguement 3 -> Please enter positive number of files per directory."
  echo "> Usage: $USAGE" $'\n'
  exit 3
fi

###################################################################
create_dirs()
{
  #mkdir $INPUT_DIR  # Create input dir
  bash testFile.sh info/virusesFile.txt info/countriesFile.txt $INPUT_DIR 0 $FILES_DIRECTORY*100
  TYPE=".txt"
  SEP="-"
  n=$(( 0 ))
  cd $INPUT_DIR
  name="France"
  #save in array the 4th column of the input file -> country names
  myarray=( $(awk '{ print $4 }' inputFile.txt) )
  #because previous line saves all the values of the 4th column,below we save only the unique country names
  sorted_unique_ids=($(echo "${myarray[@]}" | tr ' ' '\n' | sort -u | tr '\n' ' '))
  
  for country in ${sorted_unique_ids[*]}; do  # Create country dirs
    #in this file I save all the lines from the input file that contain the country name in the for loop
    mkdir $country
    cd $country
    NAME_FILE1=$country$SEP$(( $n ))$TYPE
    touch $NAME_FILE1
    grep -E $country ../inputFile.txt > $NAME_FILE1
    #create FILES_DIRECTORY text files in the directory of the country of the for loop
    for (( m = 0; m < FILES_DIRECTORY; m++ )); do  # Create date files
      NAME_FILE=$country$SEP$(( $m + 1 ))$TYPE
      touch $NAME_FILE
      res=`awk NR%$FILES_DIRECTORY==$m $NAME_FILE1`
      echo $"${res[*]}" >> $NAME_FILE
    done
    rm $NAME_FILE1
    cd ..
    
  done
  cd ..  # Return to the initial dir 
}

############################ Main ############################## 
#read_viruses
#read_countries
#create_file
create_dirs
exit 0
################################################################