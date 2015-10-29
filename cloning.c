/* cloning.c */

#include <stdio.h>
#include <string.h>		/* For strerror,strlen */
#include <stdlib.h> 	/* For exit */
#include <pthread.h> 	/* For threads */

#include <sys/socket.h>	/* For sockets */
#include <sys/types.h>
#include <netinet/in.h> /* For sockaddr_in */
#include <arpa/inet.h> 	/* For hton */
#include <netdb.h> 		/* For get host by addr */
#include <unistd.h> 	/* For read , write , close */
#include <errno.h>  	/* For errno in accept() */
#include "cloning.h"
#include "functions.h"

#define BACKLOG 5 			/* Max number of connections queued waiting to be accepted */
#define MAX_BLOOM_DIGITS 10 /* To convert long size of bloom to string in order to write it to socket */
#define BUF_SIZE 4026

extern char* bloomArray;
extern pthread_mutex_t* mutexes; 
extern int found;
extern long int size;
extern int threadsNum;
extern int loops;
extern int port;
extern char* logfile;
extern int k;
extern int seed;
extern long int sections;

int sock;

void *connection_func(void *socketPtr) {
 
	int socket = *(int*)socketPtr; 
	int sending_int, send_string_size;
	char send_size_buffer[MAX_BLOOM_DIGITS];
	
	char buffer[BUF_SIZE]; 		/* To copy bloom */
	long int bytes_written=0;
	char* localBloomArray;
	long int s=0; 				/* Sections iterator */
	char answer;

	printf("Thread %lu connecting to socket with fd:%d\n",(unsigned long int)pthread_self,socket);
	printf(" Writing arguments to socket\n");
	sprintf(send_size_buffer,"%ld",size);
	
	send_string_size = htonl(strlen(send_size_buffer)+1);
	write_wrapper(socket,&send_string_size,sizeof(send_string_size));
	write_wrapper(socket,send_size_buffer,strlen(send_size_buffer)+1);

	sending_int = htonl(threadsNum);
	write_wrapper(socket,&sending_int,sizeof(sending_int));
	
	sending_int = htonl(loops);
	write_wrapper(socket,&sending_int,sizeof(sending_int));
	
	sending_int = htonl(port);
	write_wrapper(socket,&sending_int,sizeof(sending_int));
	
	send_string_size = htonl(strlen(logfile)+1);
	write_wrapper(socket,&send_string_size,sizeof(send_string_size));
	write_wrapper(socket,logfile,strlen(logfile)+1);

	sending_int = htonl(k);
	write_wrapper(socket,&sending_int,sizeof(sending_int));

	sending_int = htonl(seed);
	write_wrapper(socket,&sending_int,sizeof(sending_int));
	printf(" Send all to socket\n");

	read_wrapper(socket, &answer, sizeof(char));	
   	printf(" Client, are all arguments correct? (y/n) %c\n",answer);
   	if (answer == 'y') {
   		
   		printf(" OK!I will send the bloom filter\n");
   		
		localBloomArray = malloc(size);
		//lock everything!
		for(s=0 ; s< sections ; s++) {
		 	pthread_mutex_lock(&(mutexes[s]));
		}
		printf(" Lock all mutexes of bloom filter\n");
		memcpy(localBloomArray,bloomArray,size);
		for(s=0 ; s< sections ; s++) {
		 	pthread_mutex_unlock(&(mutexes[s]));
		}
		printf(" Unock all mutexes of bloom filter\n");
		while(bytes_written < size) {

			bytes_written += write_wrapper(socket,localBloomArray,sizeof(buffer));
		}
		printf(" Total bytes of bloom filter written %ld\n",bytes_written);
		free(localBloomArray);	
	}

	printf("Thread %lu is closing socket with fd:%d\n",(unsigned long int)pthread_self,socket);
    close(socket);	
 
	pthread_exit(NULL);
}

int server_routine(int* portPtr) { 

	int sock_client;
	
	int port = *portPtr;
	int* new_sockPtr = malloc(sizeof(int));
	struct sockaddr_in server, client;
	struct sockaddr* clientptr = (struct sockaddr*)&client;
	struct sockaddr* serverptr = (struct sockaddr*)&server;
	socklen_t client_len;

	pthread_t client_tid;		
	int err,status; 

	printf("Server: Create socket to port:%d\n",port);
	/* Create passive communication endpoint - listener socket */
	if ( (sock = socket(PF_INET,SOCK_STREAM,0) ) == -1) { 
		perror_exit( "Socket creation failed!");
 	}
 	
	/* Create a sockadrr_in address for server */
	server.sin_family = AF_INET;					/* Internet domain */
	server.sin_addr.s_addr = htonl(INADDR_ANY); 	/* Any address */
	server.sin_port = htons(port); 					/* The given port */
	
	/* Bind socket to address */
	if (bind(sock,serverptr,sizeof(server)) < 0) {
		perror_exit("bind");
	}

	/* Listen for connections */
 	if (listen(sock,BACKLOG) < 0) {
		perror_exit ("listen");
	}
	printf ("Server: Listening port %d for connections via socket with fd %d\n",port,sock);
	
	client_len = sizeof(struct sockaddr_in);

	while (1) { 

		/* Accept connection */
		if (( sock_client = accept(sock,clientptr,&client_len))< 0) {
			//perror_exit ("accept");
			if (errno == EINVAL) { /* Last searching thread shut down socket */
				break;
			}
			perror("accept");
			continue; 
		} 
		printf("Server: Accepted connection to new socket with fd %d\n",sock_client );
		
		printf("Server: Spawn a thread to handle communication with client\n");
        
        *new_sockPtr = sock_client;
        /* Spawn  a thread */
        if( pthread_create(&client_tid,NULL,connection_func,(void*)new_sockPtr) < 0) { //WRITERS
            perror_exit("pthread_create client");
        }
        /* Wait thread to finish copying */
        if ( (err = pthread_join (client_tid,(void **) &status)) ) { //NULL
			perror_thread("pthread_join",err); //termination
         }
         printf("Server: Thread  %lu joined!\n",(unsigned long)client_tid);
         printf("Server: Closing socket with fd %d\n",sock_client);
         close(sock_client); /* parent closes socket to client */
	}
	free(new_sockPtr);
	return 0;


}
int client_routine(char* address,int port2) {
	
	int sock1;

	struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem;
    char yes = 'y';
    char no = 'n';

    if ((sock1 = socket(PF_INET, SOCK_STREAM, 0)) < 0)  {//i ==-1
    	perror_exit("socket");
    }
	//Find server address
    if ( (rem = gethostbyname(address)) == NULL ) {	
	   herror("gethostbyname"); exit(1);
    }
    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);         /* Server port */

    /* Initiate connection */
    if ( (connect(sock1, serverptr, sizeof(server))) < 0) {
	   perror_exit("connect");
	}
    printf("Client: Connected to %s, port %d via socket with fd %d\n",address, port,sock1);
  		
  	if(checkArguments(sock1,address,port2)) {
		write_wrapper(sock1,&no,sizeof(char));

		printf("Client: Closing socket with fd %d\n",sock1);
  		close(sock1);  
  		return 1;
  	}
  	else {
		printf("Client: All arguments correct!\n");

		write_wrapper(sock1,&yes,sizeof(char));
		
		printf("Client: Will copy bloomfilter!\n");
		copyBloom(sock1);
		
		printf("Client: Closing socket fd %d\n",sock1);
		close(sock1);  
		return 0;
  	}

}	

 int checkArguments(int sock1,char* address,int port2) {

 	int received_int,received_string_size;

    long int clientSize;
    char* clientSizeBuffer;
    int clientThreadsNum, clientLoops, clientPort, clientK, clientSeed;
    char* clientLogfile;

    printf("Client: Received ");
  	/* Read bloom size */
    read_wrapper(sock1, &received_int, sizeof(int));	
   	received_string_size = ntohl(received_int);
   	clientSizeBuffer = malloc(received_string_size*sizeof(char)); 
   	//printf("Received bloom size as string of length %d\n",received_string_size);
   	read_wrapper(sock1, clientSizeBuffer,received_string_size*sizeof(char));
   	//printf("Received clientSizeBuffer %s\n",clientSizeBuffer);
   	clientSize = atol(clientSizeBuffer);
   	printf("size %ld ",clientSize);

   	/* Read threads number */
  	read_wrapper(sock1, &received_int, sizeof(int));	
   	clientThreadsNum = ntohl(received_int);
   	printf("threadsNum %d ",clientThreadsNum);

   	/* Read max thread loops */
   	read_wrapper(sock1, &received_int, sizeof(int));	
   	clientLoops = ntohl(received_int);
   	printf("loops %d ",clientLoops);

   	/* Read port */
   	read_wrapper(sock1, &received_int, sizeof(int));	
   	clientPort = ntohl(received_int);
   	printf("port %d ",clientPort);

    /* Read logfile */
    read_wrapper(sock1, &received_int, sizeof(int));	
   	received_string_size = ntohl(received_int);
   	clientLogfile = malloc(received_string_size*sizeof(char));
   	//printf("logfile length %d ",received_string_size);
   	read_wrapper(sock1, clientLogfile,received_string_size*sizeof(char));
   	printf("logfile %s ",clientLogfile); 	
   		
	/* Read number of functions (k) */
    read_wrapper(sock1, &received_int, sizeof(int));	
   	clientK = ntohl(received_int);
   	printf("k %d ",clientK);

   	read_wrapper(sock1, &received_int, sizeof(int));	
   	clientSeed = ntohl(received_int);
   	printf("seed %d\n",clientSeed);  	

   	/* Check all arguments */
   	if( clientSize != size) {
   		free(clientSizeBuffer);
   		printf("Client: Closing socket with fd %d\n",sock1);
    	close(sock1);                 
    	return 1;
   	}
   	if( clientThreadsNum != threadsNum) {
   		printf("Client: Closing socket with fd %d\n",sock1);
    	close(sock1);                 
    	return 1;
   	}
   	if( clientLoops != loops) {
   		printf("Client: Closing socket with fd %d\n",sock1);
    	close(sock1);                 
    	return 1;
   	}
   	if( clientPort != port) {
   		printf("Client: Closing socket with fd %d\n",sock1);
    	close(sock1);                 
    	return 1;
   	}
   	if(strcmp(clientLogfile,logfile) == 0) { 
   		/* If client is on the same machine,logfiles must have different names */
   		if(port2 != 0) { 
   			free(clientLogfile);
   			printf("Client: Closing socket with fd %d\n",sock1);
    		close(sock1);                 
    		return 1;
   		}
   	}
   	else {
   		/* If client is on a remote machine logfiles must have tha same name */
   		if(port2 == 0) { 
   			free(clientLogfile);
   			printf("Client: Closing socket with fd %d\n",sock1);
    		close(sock1);                 
    		return 1;
   		}
   	} 
   	if( clientK != k) {
   		printf("Client: Closing socket  with fd %d\n",sock1);
    	close(sock1);                
    	return 1;
   	}   
   	if( clientSeed != seed) {
   		printf("Client: Closing socket with fd %d\n",sock1);
    	close(sock1);                 
    	return 1;
   	}   
   	free(clientSizeBuffer);
   	free(clientLogfile);

    return 0;
 }		

int copyBloom(int sock1) {

	long int bytes_read =0;
	char buffer[BUF_SIZE];

	bloomArray = malloc(size);
	if(bloomArray == NULL) {
		perror_exit("Client: malloc bloomArray");
	}		
	while(bytes_read < size) {

		bytes_read += read_wrapper(sock1,bloomArray+bytes_read, sizeof(buffer));

	}
	printf("Client: Total bytes of bloom filter read %ld\n",bytes_read);	
	
	return 0;
}
