#include <stdio.h>
#include <string.h>		/* For strerror,strlen */
#include <stdlib.h> 	/* For exit */
#include <pthread.h> 	/* For threads */
#include <limits.h>		/* For CHAR_BIT */
#include <time.h>		/* For CPU time */
#include <inttypes.h> 

#include "oracle.h"
#include "hash.h"
#include "bloomFilter.h"
#include "stack.h"


#define TIMES 25					/* Check every <TIMES> calls flag found */

extern char* bloomArray; 			/* Bloom Array is used as an array of BITS */
extern pthread_mutex_t* mutexes; 	/* Mutexes for sychronization of threads */
extern pthread_mutex_t found_mutex;
extern long int sections;
extern int section_bytes;
extern int found;


int search(long int bloom_size,int k,Stack* stack,long int* total_words,long int* words_in_bloom) {
		
	const char* str;		/*Word to be sent to oracle*/
	char** results;			/*Results of oracle(str)*/
	int i=0;
	int t=0;
	int steps=0;
	int found_local = 0;
	int end=0;
	int found2 = 0;

	/*Check Stack*/
	//printf("\nSTEP %d\n",steps);
	//printf("Size: %ld\n",stack->size);
	//while(steps <4){
	while((!end) && (!found2)) {
		
		while(t < TIMES) { /* Every <TIMES> times check found to see if another thread found word */
			
			/*If there are no more words to examine in stack,word not found!*/
			if(stack->size == 0) { 
					//printf("Thread %lu: Total words searched %ld of which %ld where in bloom\n",(unsigned long)pthread_self(),total_words,words_in_bloom);
					end = 1;
					break;
			} 
			else {
				
				if(stack->top != NULL ) {
					/*Take the next word to examine from stack*/
					str = pop(stack); 
				}
				else {
					fprintf(stderr,"Size of stack < 0!\n");
					exit(EXIT_FAILURE);
				}						
				
				
				if(!checkBloom(str)) { 		/* If word not in bloom */
					
					results = (char**)oracle(str); 					/* Ask oracle about this word */	

					if(results == NULL) { /*str is the secret word!*/
						pthread_mutex_lock(&found_mutex);
						found =1;	
						pthread_mutex_unlock(&found_mutex);	
						found2=1;
						
						printf("Thread %lu will stop : SECRET WORD FOUND!\n",(unsigned long)pthread_self());
						printf("%s\n","***************************************\0");
						print(str);						
						printf("%s\n","***************************************\0");
						free((void*)str);
						str = NULL;
						free(results);
						break;
				 	} 
					else { 
						insertBloom(str);
						
						/*Push children  into stack to examine them later*/
						i=0;
						while(results[i] != NULL) { /* Array always ends with null */						
							
								push(stack,results[i]); 				
								i++;
						}
						free(results);
						results = NULL;
					}
				}
				else {
					(*words_in_bloom)++;
				}
				(*total_words)++;
				free((void*)str); /* Free memory of str allocated in pop() */
				str = NULL;
				t++;
			}
		
		}//t
		if(end) {
			return 0;
		}
		if(found2){
			return 1;
		}	

		pthread_mutex_lock(&found_mutex);
		if(found) {
			found_local=1;
		}
		else {
			t=0;		
		}
		pthread_mutex_unlock(&found_mutex);
		if(found_local) {
			printf("Thread %lu will stop!Word Found in another thread!\n",(unsigned long)pthread_self());		
			return 2;
		}
	}
	
}

/* For debugging */
char** my_oracle(const char* str) {
	static int counter=0;
	if(counter < 5) {
		counter++;
		return 	(char**)oracle(str);
	}
	return NULL;
}

