# System_Programming_Projects
Vaccine Monitor app implemented in C with system Programming techniques.Projects implemented as part of the course Syspro K24
 
## Project 1
### Create input dir:
```bash
bash testFile.sh info/virusesFile.txt info/countriesFile.txt input_dir x y
```
where ,x=0|1 ,0 doesn't allow duplicates while 1 does. y is the numer of records you wish to create eg.:1000

## Compile
### To create new input dir and compile the program simultaneously, write in your terminal:
```bash
make all DUP_FLG=x RECORDS=y
```
where ,x and y are used as described above
### To compile ony the project:
```bash
make
```
### To delete the objective files:
```bash
make clean
```
### To run the project and check memory:

```bash
valgrind --leak-check=full ./vaccineMonitor -c input_dir/inputFile.txt -b bloomsize
```

## Project 2
### Create input dir:
```bash
bash ./create_infiles.sh input_dir/inputFile.txt input_dir2 numfiles
```
where numfiles equals to the number of files per country dir

## Compile
### To create new input dir and compile the program simultaneously, write in your terminal:
```bash
make script NUM_FILES_PER_DIR=x
```
where numfiles equals to the number of files per country dir
### To compile ony the project:
```bash
make
```
### To delete the objective files:
```bash
make clean
```
### To run the project and check memory:

```bash
valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes -s ./travelMonitor -m 3 -b 100 -s 1000 -i input_dir
```

## Project 3
### Create input dir:
```bash
bash ./create_infiles.sh input_dir/inputFile.txt input_dir2 numfiles
```
where numfiles equals to the number of files per country dir

## Compile
### To create new input dir and compile the program simultaneously, write in your terminal:
```bash
make script NUM_FILES_PER_DIR=x
```
where numfiles equals to the number of files per country dir
### To compile ony the project:
```bash
make
```
### To delete the objective files:
```bash
make clean
```
### To run the project and check memory:

```bash
valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes ./travelMonitorClient -m 3 -b 100 -c 2 -s 1000 -i input_dir -t 3 < instructions.txt
```
