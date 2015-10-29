#include <stdio.h>
#include <stdlib.h> 	
#include <string.h>		/* For strerror,strlen */
#include <pthread.h> 	/* For threads */
#include <time.h>		/* For CPU time */
#include <inttypes.h>
#include <sys/socket.h>	/* To shutdown socket */

#include "oracle.h"
#include "hash.h"
#include "bloomFilter.h"
#include "stack.h"
#include "cloning.h"
#include "functions.h"

#define MAX_WORD_LENGTH 12
#define MIN_SECTION_BYTES 64

extern int sock;

/* Global variables */

char* bloomArray; 			/* Bloom Array is used as an array of BITS */
							/*** Command-line arguments ***/
long int size;				/* Size of Bloom Array in BYTES*/
int threadsNum;				/* Number of threads */
int loops;					/* Number of loops threads repeats searching */
int port;					/* Port waiting for clients' requests */
char* logfile;				/* Filename with statistics by each thread */
int k; 						/* Number of hashNumberh functions */

pthread_mutex_t* mutexes; 	/* Mutexes for sychronization of searching threads */

FILE* fp;
pthread_mutex_t logfile_mutex;

int found; 					/* Flag for word found */
pthread_mutex_t found_mutex; 

int threadsCounter=0;
pthread_mutex_t counter_mutex;

long int sections;
int section_bytes;

int seed;					/* Seed for initSeed.Will be given by user*/

/* Forward declarations */
int search(long int size,int hashNumber,struct Stack*tempStack,long int* total_words,long long * words_in_bloom);//TODO vgale ta global
void manamgeArguments(int argc,char* argv[],char** address,int* port2);

/* Thread function */
void* thread_func(void* data) { 
	
	Stack* tempStack;
	int result,l=0;
	long int total_words=0;
	long long words_in_bloom=0;
	char* word;
	long double per;	

	tempStack = createStack();

	while(l < loops) {
		
		word = wordGenerator(MAX_WORD_LENGTH);

		printf("Thread %lu searching for word %s\n",(unsigned long)pthread_self(),word);
		push(tempStack,word);
		
		result = search(size,k,tempStack,&total_words,&words_in_bloom); 
		

		if(result == 0) { /*If secret word found*/	 
			printf("Thread %lu,Loop %d:Word not Found!\n",(unsigned long)pthread_self(),l);
			l++;
		}
		else if (result) {
			//printf("Thread %lu found(out)\n",(unsigned long)pthread_self());
			break;
		}
		else if(result == 2) {
			printf("Thread %lu returbed because another found\n",(unsigned long)pthread_self());
			
		}
	}
	/* Find percentage */
	words_in_bloom = words_in_bloom*100; 
	per = words_in_bloom/total_words;
	
	/* Write statistics */
	pthread_mutex_lock(&logfile_mutex);
	fprintf(fp,"Thread  %lu searched: %10ld words  of which  %2.2Lf%% were in bloom filter.\n",(unsigned long)pthread_self(),total_words,per);
	pthread_mutex_unlock(&logfile_mutex);

	destroyStack(tempStack);
	
	pthread_mutex_lock(&counter_mutex);
	threadsCounter++;
	/* Last thread is closing the socket */
	if ( threadsCounter == threadsNum ) { 
		shutdown(sock,SHUT_RDWR);
		printf("Last thread %lu closing socket with fd %d\n",(unsigned long)pthread_self(),sock);
	}	
	pthread_mutex_unlock(&counter_mutex);
	return 0;
}


int main (int argc, char *argv[]) {
			
	int port2=0;			/* Port used if programs run on the same machine*/
	char* address=NULL;							
	int plus_section =-1;
	int i=0;
	
	pthread_t* tids ;		/* Store ids of searching threads */
	int err,status;
	
	clock_t begin, end;		/* To calculate program execute time */
	double time_spent;

	found = 0;

	manamgeArguments(argc,argv,&address,&port2);
	printf("\nSearching word...\nSize:%ld N:%d L:%d Port:%d Logfile:%s k:%d",size,threadsNum,loops,port,logfile,k);
	if(address != NULL) {
		printf(" address:%s",address);
		if(port2 == 0) {
			port2 = port;
		}
		printf(" port2:%d",port2);
	}
	printf("\n");
	
	/* Initialize oracle */	
	printf("Please give an integer as seed:");
	if(scanf(" %d",&seed) != 1) {
		perror_exit("No integer entered!\n");
	}

	//setEasyMode();
	initSeed(seed);	
	initAlloc(malloc);

	begin = clock();
	
	/* Client */
	if(address != NULL){ 
		/* Check arguments and copy bloom filter */
		if (client_routine(address,port2)) {
			free(address);
			free(logfile);
			printf("Client: Exiting..arguments didn't match!\n");	
			//pthread_exit (NULL) ;
			exit(EXIT_FAILURE);
		}
	}
	/* Server */
	else {
		/* Create Bloom Filter */
		bloomArray = malloc(size);
		if(bloomArray == NULL) {
			perror_exit("main:malloc of bloomArray");
		}
	}

	
	/* Divide bloom filter into sections */
	sections = size/MIN_SECTION_BYTES;
	section_bytes=MIN_SECTION_BYTES;	

	i=0;
	while(sections > size/sections) {
		i++;
		section_bytes = i*MIN_SECTION_BYTES;
		sections = size/section_bytes;
	}

	if(size%section_bytes !=0 ) {
		plus_section = (size-sections*section_bytes);
	}	
	printf("Bloom Filter will be divided in %ld section(s) of %d bytes\n",sections,section_bytes);
	if(plus_section != -1 ) {
		printf("plus 1 of %d bytes\n",plus_section);
		sections++;
	}
	printf("\n");


	if (( fp = fopen (logfile,"w") )  == NULL ) {			
			perror ("open logfile"); 
	}

	/* Initialize mutexes */
	pthread_mutex_init(&(found_mutex),NULL);
	pthread_mutex_init(&(logfile_mutex),NULL);
	pthread_mutex_init(&(counter_mutex),NULL);

	mutexes = malloc(sections*sizeof(pthread_mutex_t));
	for(i=0 ; i< sections ; i++) {
		 pthread_mutex_init(&(mutexes[i]),NULL);
	}

	/* Store threads ids */
	if (( tids = malloc (threadsNum* sizeof(pthread_t))) == NULL ) {
		perror ( "malloc:threads ids " ) ;
		exit (EXIT_FAILURE) ;
	}
	
	/* Create threads  */
	for(i=0 ; i<threadsNum ; i++) {

		if ( (err = pthread_create(tids+i, NULL, thread_func,(void *)5) )) { //TODO args
			perror_thread("pthread_create",err); 
			exit(EXIT_FAILURE) ;
		}
	}

   	if (address != NULL) { 
		port = port2; 		/* Client became server.Other clients must connect to the second port given if on same machine */
	}

	server_routine(&port); 	/* Main thread blocks in accept of socket waiting for connections */
	
	for(i=0 ; i<threadsNum ; i++) {
		if ( (err = pthread_join ( *(tids+i),(void **) &status)) ) { 
			perror_thread("pthread_join",err);
			exit (EXIT_FAILURE) ;
		}		
	}
	
	for(i=0 ; i<sections ; i++) {
		if ( (err = pthread_mutex_destroy(&(mutexes[i]) )) ) {
			perror_thread(" pthread_mutex_ destroy",err ); 
			exit(EXIT_FAILURE) ;
		}
	}

	fclose(fp);
	free(mutexes);
	free(tids);
	free(bloomArray);
	free(logfile); 
	if(address !=NULL)
		free(address);

	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Program execute time %f\n",time_spent);
		
	exit(EXIT_SUCCESS);
}

void manamgeArguments(int argc,char* argv[],char** address,int* port2) {

	if (argc < 6) {
		fprintf(stderr,"Not enough argumnets!\nUsage : %s SIZE N L PORT LOGFILE [-k NUM] [-h ADDRESS [PORT2] ]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	else {
		size = atol(argv[1]);
		threadsNum = atoi(argv[2]);
		loops = atoi(argv[3]);
		port = atoi(argv[4]);
		logfile = malloc(strlen(argv[5])+1); //+1?
		if(logfile == NULL) {
			perror_exit("malloc logfile");
		}
		strcpy(logfile,argv[5]);
		k = 3; 	/* default number of hash functions */
	}
	if((argc == 8 ) || (argc == 9)) { /* one optional argument with its option*/
		if(strcmp(argv[6],"-k") == 0 ) {		
			k =atoi(argv[7]);
		}
		else if(strcmp(argv[6],"-h") == 0 ) {
			*address = malloc(strlen(argv[7])+1);
			if(*address == NULL) {
				perror_exit("malloc address");
			}
			strcpy(*address,argv[7]);
			if(argc == 9) {
				*port2 = atoi(argv[8]);
			}
		}
		else {
			printf("whyyy\n");
			fprintf(stderr,"Usage : %s SIZE N L PORT LOGFILE [-k NUM] [-h ADDRESS [PORT2] ]\n",argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	else if ( ((argc == 10) || (argc == 11)) ) { 	/* two optional arguments with their options*/
		if(strcmp(argv[6],"-k") == 0 ) {			/* -k is first in order */
			k =atoi(argv[7]);
		}
		else if(strcmp(argv[6],"-h") == 0 ) {		/* -h is first in order */
			*address = malloc(strlen(argv[7])+1);
			if(*address == NULL) {
				perror_exit("malloc address");
			}
			strcpy(*address,argv[7]);
			if(argc == 11) {
				*port2 = atoi(argv[8]);
			}
		}
		else {
			fprintf(stderr,"Usage : %s SIZE N L PORT LOGFILE [-k NUM] [-h ADDRESS [PORT2] ]\n",argv[0]);
			exit(EXIT_FAILURE);

		}
		if(strcmp(argv[8],"-h") == 0 ) {				/* -h is second*/	
			*address = malloc(strlen(argv[9])+1);
			if(*address == NULL) {
				perror_exit("malloc address");
			}
			strcpy(*address,argv[9]);
			if(argc == 11) {
				*port2 = atoi(argv[10]);
			}
		}
		else if ((strcmp(argv[8],"-k") == 0) || (strcmp(argv[9],"-k") == 0  ) ) {
			if(argc == 10) {
				if(strcmp(argv[8],"-k") == 0 ) {		/*-k is second*/
					k =atoi(argv[9]);
				}
			}		
			else{
				if(strcmp(argv[9],"-k") == 0 ) {		/*-k is second,and address has port*/
					k =atoi(argv[10]);
				}
			}
		}
		else {
			fprintf(stderr,"Not enough argumnets!\nUsage : %s SIZE N L PORT LOGFILE [-k NUM] [-h ADDRESS [PORT2] ]\n",argv[0]);
			exit(EXIT_FAILURE);
		}	

	}
	if( (argc != 6) && (argc != 8) &&( argc !=9)&& (argc != 10) && (argc != 11) ){
		fprintf(stderr,"Not enough argumnets!\nUsage : %s SIZE N L PORT LOGFILE [-k NUM] [-h ADDRESS [PORT2] ]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
		
	if (size < MIN_SECTION_BYTES) { 
		perror_exit("SIZE of Bloom Filter must be at least 64 bytes!\n");
	}

}


