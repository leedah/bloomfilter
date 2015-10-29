#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#define perror_thread(s,e) fprintf (stderr ," %s : %s \n",s,strerror(e))

void perror_exit (char* message);
char* wordGenerator(int length);

int write_wrapper(int fd, void *buffer, size_t size);
int read_wrapper(int fd, void *buffer, size_t size);

#endif
