/* bloomFilter.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

#include <pthread.h> 				/* For threads */

#include "oracle.h"
#include "hash.h"
#include "bloomFilter.h"

extern char* bloomArray; 			/* Bloom Array is used as an array of BITS */
extern int k;
extern long int size;
extern pthread_mutex_t* mutexes;
extern long int sections;
extern int section_bytes;

/*Prints printable characters os a string,otherwise prints their ASCII code*/
void print(const char* str) {

	char c;
	int i;
	for(i=0; str[i] != 0 ; i++ ) { 
		c = str[i];
		if((c >32) && (c < 127)) {
			printf("%c",c);
		}
		else {  printf("'\\%d'",c); }
	}
	printf("\n");
}

/* Prints size bytes of Bloom Array */
void printBloom(char* bloom,int size)
{

	int bit,i,b;
    char c;
       
    for (i=0; i < size; i++)
    {
        printf("\nbyte %d : ",i);
        c = bloom[i];

         for (bit=0; bit < CHAR_BIT; bit++)
         {
         	b = c & 0X01;					
            printf("%i", b);				
            c = c >> 1; 						
 		}
    }
	 printf("\n");
}

void insertBloom(const char*word) {

	uint64_t value,mask;
	long int pos;
	int i,j,imin,tmp;
	uint64_t bytes[k];
	uint64_t bits[k];

	long int active_sections[k];
    char* bloom = bloomArray;
    long int bloom_size = size;


	/* Find all mutex areas of positions returned by the hash functions */
	for(i=0 ; i<k ; i++ ) { 
		
		/* Find hash value return by i-hash */
		value = hash_by(i,word);
		
		/* Find position in bloomFilter */
		pos = (long int) (value % (bloom_size*CHAR_BIT)); 
		////printf("pos %" PRIu64 "\n",pos);
		//printf("-------\npos %ld \n",pos); //THIS
		//printf("Bloom bits %ld\n",bloom_size*CHAR_BIT);
		
		/* Find section of this position */
		bytes[i] = pos/CHAR_BIT;
		bits[i]  = pos % CHAR_BIT;
		//printf("byte %" PRIu64 "\n",bytes[i]); //THIS
		//printf("bit %" PRIu64 "\n",bits[i]); //THIS
		active_sections[i] = bytes[i]/section_bytes;
		//printf("Hash %d:will insert in section %ld\n",i,active_sections[i]);
	}
    /* Sort sections */
    for (i=0 ; i<k-1 ; i++){
        imin = i;
        for (j = i+1; j < k ; j++){
            if (active_sections[j] < active_sections[imin]){
                imin = j;
            }
        }
        tmp = active_sections[i];
        active_sections[i] = active_sections[imin];
        active_sections[imin] = tmp;
    }
    i=0;
    while(i < k) {
    	//printf("Trying to locking active_sections[%d] = %d\n",i,active_sections[i]);
    	pthread_mutex_lock(&mutexes[i]);
    	//printf("section %ld LOCKED!\n",active_sections[i]);
    	j=i+1;
    	while(j < k) {
    		if(active_sections[i] == active_sections[j]) {
    			i++;
    			j++;
    		}
    		else { break;}
    	}
    	i++;
    }
    for(i=0 ; i<k ; i++) {

    	mask =  1 << bits[i];   	
		bloom[bytes[i]] = bloom[bytes[i]] | mask; 	//Bitwise OR
	}
	 i=0;
    while(i < k) {
    	//printf("Trying to unlock active_sections[%d] = %d\n",i,active_sections[i]);
    	pthread_mutex_unlock(&mutexes[i]);
    	//printf("section %ld UNLOCKED!\n",active_sections[i]);
    	j=i+1;
    	while(j < k) {
    		if(active_sections[i] == active_sections[j]) {
    			i++;
    			j++;
    		}
    		else { break;}
    	}
    	i++;
    }
}

int checkBloom(const char*word) {

	uint64_t value,mask;
	long int pos;
	int i,j,imin,tmp;
	int b;
	long int active_sections[k];
	uint64_t bytes[k];
	uint64_t bits[k];
    char* bloom = bloomArray;
    long int bloom_size = size;


    b = -1;

	/* Find all mutex sections of positions returned by the hash functions */
	for(i=0 ; i<k ; i++ ) { //for each hash function
		
		/* Find hash value return by i-hash */
		value = hash_by(i,word);
		//printf("hash value %" PRIu64 "\n",value);
		
		/* Find position in bloomFilter */
		pos = (long int) (value % (bloom_size*CHAR_BIT)); 
		//printf("pos %" PRIu64 "\n",pos);
		//printf("-------\npos %ld \n",pos);
		//printf("Bloom bits %ld\n",bloom_size*CHAR_BIT);
		
		/* Find section of this position */
		bytes[i] = pos/CHAR_BIT;
		bits[i]  = pos % CHAR_BIT;

		//printf("byte %" PRIu64 "\n",byte);
		active_sections[i] = bytes[i]/section_bytes;
		//printf("Hash %d:will check section %ld\n",i,active_sections[i]);
	}
	/* Sort sections */
    for (i=0 ; i<k-1 ; i++){
        imin = i;
        for (j = i+1; j < k ; j++){
            if (active_sections[j] < active_sections[imin]){
                imin = j;
            }
        }
        tmp = active_sections[i];
        active_sections[i] = active_sections[imin];
        active_sections[imin] = tmp;
    }
    
    i=0;
    while(i < k) {
    	//printf("Trying to locking active_sections[%d] = %d\n",i,active_sections[i]);
    	pthread_mutex_lock(&mutexes[i]);
    	//printf("section %ld LOCKED!\n",active_sections[i]);
    	j=i+1;
    	while(j < k) {
    		if(active_sections[i] == active_sections[j]) {
    			i++;
    			j++;
    		}
    		else { break;}
    	}
    	i++;
    }
    for(i=0 ; i<k ; i++) {
		
    	mask =  1 << bits[i];    	
		b = ((bloom[bytes[i]] & mask) != 0);
		//printf("Bit:%d\n",b);
		
		/* if you find at least one 0 then for sure the word is not in Bloom*/
		if(!b){ 
			break;
		}
	}

	i=0;
    while(i < k) {
    	//printf("Trying to unlock active_sections[%d] = %d\n",i,active_sections[i]);
    	pthread_mutex_unlock(&mutexes[i]);
    	//printf("section %ld UNLOCKED!\n",active_sections[i]);
    	j=i+1;
    	while(j < k) {
    		if(active_sections[i] == active_sections[j]) {
    			i++;
    			j++;
    		}
    		else { break;}
    	}
    	i++;
    }
    if(b ==0) {
    	return 0; /* if there is at least one bit 0,word is not in bloom */
    }
    return 1;
}

































