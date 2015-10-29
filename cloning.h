#ifndef CLONING_H_
#define CLONING_H_

int server_routine(int* port);
int client_routine(char* address,int port2);
int checkArguments(int sock,char* address,int port2);
int copyBloom(int sock1);
#endif