#Aristi Papastavrou
#AM:1115201800154

PROGRAM = vaccineMonitor

MAKE-DIR = $(PWD)

SRC := $(MAKE-DIR)/src
INCLUDE := $(MAKE-DIR)/includes

CFLAGS := -g3 -I $(INCLUDE)
LIBS := -lm
CC := gcc

OBJS := ./main.o ./src/bloom.o ./src/CitizenHash.o ./src/hashes.o ./src/MyHTable.o ./src/MyLinked_List.o  ./src/Queries.o ./src/Skip_Lists.o ./src/Vaccine_Monitor_Funcs.o ./src/Utilities.o

SCARGS = $(DUP_FLG) $(RECORDS)

SCRIPT = testFile.sh

sourceA = info/virusesFile.txt

sourceb = info/countriesFile.txt

dir = input_dir

export MAKE-DIR CC CFLAGS OBJS LIBS

$(PROGRAM): clean $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM) $(LIBS)


all: $(PROGRAM)
	bash $(SCRIPT) $(sourceA) $(sourceb) $(dir) $(SCARGS)
	
	
.PHONY : clean
clean :
	@$(RM) -f  $(PROGRAM) $(OBJS)
	
#.PHONY : script
#script :
#	bash $(SCRIPT) $(sourceA) $(sourceb) $(dir) $(SCARGS)
	
