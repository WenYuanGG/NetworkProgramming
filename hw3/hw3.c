#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MAXTHREADS 4

char *message[MAXTHREADS+1] = {
	"Hello 0",
	"Hello 1",
	"Hello 2",
	"Hello 3",
	"Error !!!" 
};

void *Hello(void *pthreadid){
	// Get id from the dynamic memory
	int id = *(int*)pthreadid;
	// Release the dynamic memory
	free(pthreadid);

	printf("My Thread's ID [%d] %s\n", id, message[id]);
}

int main(int argc, char *argv[]){
	pthread_t child[MAXTHREADS];
	int id;
	int i;

	for(id=0; id<MAXTHREADS; id++){
		// Allocate a dynamic memory
		int *thisParam = malloc(sizeof(int));

		// Put id into dynamic memory
		*thisParam = id;

		printf("Create thread %d\n", id);

		// Create a new thread
		pthread_create(&child[id], NULL, Hello, thisParam);
	}

	// Wait for each thread to finish executing
	for(i=0; i<MAXTHREADS; i++){
		pthread_join(child[i], NULL);
	}

	return 0;
}