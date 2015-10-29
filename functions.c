/* functions.c */
#include <stdio.h>
#include <string.h>		/* For strerror,strlen */
#include <stdlib.h> 	/* For exit */
#include <pthread.h> 	/* For threads */

#include <unistd.h> 	/*read , write , close */
#include <errno.h>      /* errors of the above */
#include "functions.h"

void perror_exit (char* message) {
	perror(message);
	exit(EXIT_FAILURE);
}

char* wordGenerator(int length) {

    static char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";        
    char *word = NULL;
    int n;
    static int i=0;
    unsigned int seed = pthread_self()+i;


    if (length > 0) {
        word = malloc(sizeof(char) * (length +1));

        if (word != NULL ) {            
            for (n =0; n < length; n++) {            
                
                int key = rand_r(&seed)%(int)(sizeof(characters) -1);
                word[n] = characters[key];
            }
            word[length] = '\0';
        }
    }
    i++;
    return word;
}

/* read() repeatedly until ’size’ bytes are read */
int read_wrapper(int fd, void *buffer, size_t size)	{
	
	size_t nread, bytes_read=0;

	while(bytes_read <= size) {
		
		nread = read(fd, buffer + bytes_read, size - bytes_read);
		if(nread == 0) {
			break;
		}
		if(nread < 0) {
			perror("read");
			break;
		}
		bytes_read += nread;
	}
	return bytes_read;
}

/* write() repeatedly until ’size’ bytes are written */
int write_wrapper(int fd, void *buffer, size_t size) {

	size_t nwrite, bytes_written;
	for(bytes_written = 0; bytes_written < size; bytes_written += nwrite) {
	
		if((nwrite = write(fd, buffer + bytes_written, size - bytes_written)) < 0)
			return -1;
	}
	return bytes_written;
   
}
