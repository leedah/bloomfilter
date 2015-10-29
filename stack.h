#ifndef STACK_H_
#define STACK_H_

	
/* Auxiliary structure stack (lifo).
 * Used to temporaliy store elements that we will examine next.
 * Also used to temporaly store pointers when traversing(insertion/print) the tree */

typedef struct StackNode {		
	char* elem; 					
	struct StackNode* prev;
}StackNode;


typedef struct Stack {
	StackNode* top;
	long size;
}Stack;

Stack* createStack();
char* pop(Stack* stack);
void push(Stack* stack,char* elem);
int destroyStack(Stack* stack);
void printStack(Stack* stack); 


#endif
