#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <syscall.h>
#include "webserver.h"

#define MAX_REQUEST 100

int port, numThread;
int j = 0;
int semret;
int mutex = 1;
int empty, full;
int buffer[100];
sem_t sem;
pthread_mutex_t lock;
sem_t semFull, semEmpty, semMutex;
int buffermarker, getbufferat;

void *listener(){ //producer
	int r;
	struct sockaddr_in sin;
	struct sockaddr_in peer;
	int peer_len = sizeof(peer);
	int sock;
	int process[numThread];
	printf("[TEST] Hi listener is created\n");
	sock = socket(AF_INET, SOCK_STREAM, 0);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	r = bind(sock, (struct sockaddr *) &sin, sizeof(sin));
	if(r < 0) {
		perror("Error binding socket:");
		return -1;
	}

	r = listen(sock, 5);
	if(r < 0) {
		perror("Error listening socket:");
		return -1;
	}

	printf("HTTP server listening on port %d\n", port);


	while(1){
		int s;
		s = accept(sock, NULL, NULL);
		if (s < 0) break;
		if(s < 0){
			printf("Accept failed.\n");
		}
//		printf("[LISTENER] Going into semaphore\n");
		
		sem_wait(&semEmpty); //empty buffer count -1;
		sem_wait(&semMutex);
		
		buffer[buffermarker] = s;
		buffermarker = buffermarker+1;

		sem_post(&semMutex);
		sem_post(&semFull); //full buffer count +1;
		//process(s); //request for client
	}//do while end
	close(sock);
}

void workermanager(){ //(consumer)
	//while loop, monitor 
	while(1){
		sem_wait(&semFull); //full --
		sem_wait(&semMutex);
	
		int s = buffer[getbufferat];
		getbufferat = getbufferat + 1;	
	
		sem_post(&semMutex);
		sem_post(&semEmpty); //empty ++
		process(s);
	}//do while end
	//pull out resource from buffer when called
	//with each instance called, sem_trywait
	//with each instance returned, sem_post
}

void thread_control()
{	
	printf("Main called thread control\n");
}//thread control
int main(int argc, char *argv[])
{
	if(argc < 3 || atoi(argv[1]) < 2000 || atoi(argv[1]) > 50000)
	{
		fprintf(stderr, "./webserver PORT(2001 ~ 49999) #_of_threads\n");
		return 0;
	}

	int rc;
	port = atoi(argv[1]);
	numThread = atoi(argv[2]);
	
	pthread_t workerthread[numThread];
	pthread_t listenerthread; //initializing` pthreads
	//empty = numThread;
	//full = 0;
	//mutex = 1;
	sem_init(&semFull, 1, 0); //initializing semaphore
	sem_init(&semEmpty, 1, 100);
	sem_init(&semMutex, 1, 1); //mutex lock
	for(int i = 0 ; i < numThread ; i++){
		printf("Creating worker thread: [tid #%d, pid #%d]\n", i,getpid());
		pthread_create(&workerthread[i], NULL, workermanager, NULL);
	}
	//printf("about to create listener\n");
	pthread_create(&listenerthread, NULL, listener, NULL);
	
	for(int i = 0; i< numThread ; i++){
		pthread_join(workerthread[i], NULL);
	}
	//pthread_join(&listenerthread, NULL);
	pthread_exit(NULL);
	if(argc > 3) 
			CRASH = atoi(argv[3]);
	if(CRASH > 50) CRASH = 50;
	thread_control();
	printf("program end\n");
	sem_destroy(&full);
	sem_destroy(&empty);
	sem_destroy(&mutex);
	return 0;
}//main end
