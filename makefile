#makefile

CC = gcc
FLAGS = -g3 -c -O2 -Wall
FLAGS1 = -lpthread
OBJS1 = main.o bloomFilter.o stack.o search.o cloning.o functions.o
HEADER = hash.h oracle.h bloomFilter.h stack.h cloning.h functions.h
OUT1 = invoke-oracle-multithreaded

all: $(OUT1)

#create/compile the individual files seperately
$(OUT1): $(OBJS1)
	 $(CC) -o $(OUT1) -g3 $(OBJS1) -L . -loracle_v3 -lhash -static $(FLAGS1)
	 	  
main.o: main.c $(HEADER)
	$(CC) $(FLAGS) main.c

bloomFilter.o: bloomFilter.c $(HEADER)
	$(CC) $(FLAGS) bloomFilter.c
	
queue.o: stack.c $(HEADER)
	$(CC) $(FLAGS) stack.c
	
search.o: search.c $(HEADER)
	$(CC) $(FLAGS) search.c

cloning.o: cloning.c $(HEADER)
	$(CC) $(FLAGS) cloning.c

functions.o: functions.c $(HEADER)
	$(CC) $(FLAGS) functions.c
	
#clean
.PHONY: clean

clean:
	rm $(OBJS1) $(OUT1)
