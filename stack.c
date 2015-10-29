/* stack.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "oracle.h"
#include "hash.h"
#include "bloomFilter.h"	 /* For print() */
#include "stack.h"
#include "functions.h"		/* For perror_exit() */

Stack* createStack() {
	
	Stack* stack = malloc(sizeof(Stack));
	stack->top = NULL;
	stack->size = 0;
	return stack;
}

/* Deletes a node and return the word.Word will be freed in calling function */	
char* pop(Stack* stack) {	
	
	char* word;
		
	StackNode* temp = stack->top;
	if ((temp == NULL) || (stack->size == 0)) {
		perror_exit("pop:Error!Stack is empty!\n");
	}
	
	if(temp->prev != NULL) { 		/* If there are at least 2 elements.. */
		stack->top = temp->prev; 	/* ..make top points to previous node */
	}
	else { 							/* There is only one and its about to be removed */
		stack->top = NULL;
	}
	
	word = temp->elem;
			
	temp->elem = NULL; 				/* For safety */
	temp->prev = NULL;
	
	free(temp);						/* Free the stack node */
	temp = NULL;
	stack->size--;
	return word;
}

/* Creates a node to store word.
*  Does note allocate new memory for the word,just assigns the element pointer
*  to existing space returned by oracle */
void push(Stack* stack,char* elem) {
	
	StackNode* node = malloc(sizeof(StackNode));/* Allocate  a stack node for the element (word)  */
	if (node == NULL) {
		perror_exit("push:allocate node");
	}
	node->elem = elem;
	if(node->elem == NULL) {
		perror_exit("push:element to be pushed is null!");
	}

    if (stack->top == NULL) {	/* If it is the first element */				
    	node->prev = NULL;
        stack->top = node;
    }
    else
    {	node->prev = stack->top;
    	stack->top = node;
    }
    stack->size++;
}

void printStack(Stack* stack) {

	const char* word;
	int i=0;

	StackNode* current = stack->top;	
	while(current != NULL) {
		printf("Stack #%d:",i);
		word = current->elem;
		print(word);
		printf(" \n");
		current= current->prev;
		i++;
	} 
}

int destroyStack(Stack* stack) {
	char* word;
	printf(stack->size>0 ? "Deleting stack of size: %ld\n":"Deleting empty stack\n",stack->size);
	while(stack->size > 0 ) {
		word = pop(stack);
		free(word);
	}
	free(stack);
	stack = NULL;
	return 0;
}
	

