
#ifndef BLOOM_FILTER_H_
#define BLOOM_FILTER_H_

/* Prints printable characters οf a string,otherwise prints their ASCII code */
void print(const char* str);										
void printBloom(char* read_buffer,int size); 
/* Inserts word in Bloom Filter (MT version) */
void insertBloom(const char*word);
/* Checks if a word is in Bloom Filter.Returns 0 if word is not in Bloom,1 otherwise(MT version) */
int checkBloom(const char*word); 	

#endif
