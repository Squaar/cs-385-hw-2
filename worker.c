/*
* Matt Dumford - mdumford
* mdumfo2@uic.edu
*
* WORKER
* 
* Reads and writes from shared memory and reports errors to master.
* Locks down shared memory with semaphores if lock is turned on.
*/

#include "message.h"

#include <errno.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define BOOL int
#define TRUE 1
#define FALSE 0

float strtof(const char *str, char **endptr); //only need this declaration on bert...?
int str2int(char *str); //convert a string to int, wrapper for strtol
float str2float(char *str);	//convert a string to float, wrapper for strtof
void wait(int sem, int semID, int nBuffers); //reserves resources for a semaphore
void signal(int sem, int semID, int nBuffers); //frees resources for a semaphore

int main(int argc, char **argv){
	if(argc < 6 || argc > 7){
		printf("Wrong number of arguments!");
		exit(-1);
	}
	
	int workerID = str2int(argv[1]);
	int nBuffers = str2int(argv[2]);
	float sleepTime = str2float(argv[3]);
	int msgID = str2int(argv[4]);
	int shmID = str2int(argv[5]);
	int semID;
	BOOL lock = FALSE;
	if(argc == 7){ //if a semID is given
		semID = str2int(argv[6]);
		lock = TRUE;
	}

	//connect to message queue
	int msgQ = msgID;
	if(msgQ == -1){
		perror("Error connecting to message queue");
		exit(-1);
	}

	//send startup message
	struct message msg;
	msg.mtype = 1;
	msg.workerID = workerID;
	msg.sleepTime = sleepTime;

	if(msgsnd(msgQ, &msg, sizeof(struct message) - sizeof(long), 0) == -1){
		perror("Error sending message");
		exit(-1);
	}

	//attatch to shared memory
	int *shm = shmat(shmID, (void *)0, 0);
	if(shm == (int *) -1){
		perror("Error attatching to shared memory");
		exit(-1);
	}
	
	//===================== PART 5 ==============================
	
	//use semaphores
	if(lock){
		int i;
		int currentBuffer = workerID;
		for(i=0; i<nBuffers; i++){
			int j;
	
			for(j=0; j<2; j++){ //read twice
				wait(currentBuffer, semID, nBuffers);
		
				int read = shm[currentBuffer];
	
				if(usleep(sleepTime*1000000) == -1){
					perror("Error sleeping");
					exit(-1);
				}
	
				//check if changed
				if(shm[currentBuffer] != read){
					msg.mtype = 3;
					msg.workerID = workerID;
					msg.changedBuffer = currentBuffer;
					msg.initVal = read;
					msg.newVal = shm[currentBuffer];
					if(msgsnd(msgQ, &msg, sizeof(struct message), 0) == -1){
						perror("Error sending message");
						exit(-1);
					}
				}

				signal(currentBuffer, semID, nBuffers);
		
				currentBuffer += workerID;
				if(currentBuffer >= nBuffers)
					currentBuffer -= nBuffers;
			}
	
			//write
			wait(currentBuffer, semID, nBuffers);

			int read = shm[currentBuffer];
	
			if(usleep(sleepTime*1000000) == -1){
				perror("Error sleeping");
				exit(-1);
			}
			
			shm[currentBuffer] = read | (1<<(workerID -1));

			signal(currentBuffer, semID, nBuffers);
	
			currentBuffer += workerID;
			if(currentBuffer >= nBuffers)
				currentBuffer -= nBuffers;
	
		}
	}

	//===================== PART 3/4 ============================

	//dont use semaphores
	else{	
		int i;
		int currentBuffer = workerID;
		for(i=0; i<nBuffers; i++){
			int j;

			for(j=0; j<2; j++){ //read twice
				int read = shm[currentBuffer];
	
				if(usleep(sleepTime*1000000) == -1){
					perror("Error sleeping");
					exit(-1);
				}
	
				//check if changed
				if(shm[currentBuffer] != read){
					msg.mtype = 3;
					msg.workerID = workerID;
					msg.changedBuffer = currentBuffer;
					msg.initVal = read;
					msg.newVal = shm[currentBuffer];
					if(msgsnd(msgQ, &msg, sizeof(struct message), 0) == -1){
						perror("Error sending message");
						exit(-1);
					}
				}
				
				currentBuffer += workerID;
				if(currentBuffer >= nBuffers)
					currentBuffer -= nBuffers;
			}
	
			//write
			int read = shm[currentBuffer];
	
			if(usleep(sleepTime*1000000) == -1){
				perror("Error sleeping");
				exit(-1);
			}
			
			shm[currentBuffer] = read | (1<<(workerID -1));
	
			currentBuffer += workerID;
			if(currentBuffer >= nBuffers)
				currentBuffer -= nBuffers;
	
		}
	}
	
	//cleanup message
	msg.mtype = 2;
	msg.workerID = workerID;
	if(msgsnd(msgQ, &msg, sizeof(struct message), 0) == -1){
		perror("Error sending message");
		exit(-1);
	}

	exit(0);
}

void wait(int sem, int semID, int nBuffers){
	struct sembuf sembuffer;
	sembuffer.sem_num = sem;
	sembuffer.sem_op = -1;
	sembuffer.sem_flg = SEM_UNDO;

	if(semop(semID, &sembuffer, 1) == -1){
		perror("Error in semop");
		exit(-1);
	}
}

void signal(int sem, int semID, int nBuffers){
	struct sembuf sembuffer;
	sembuffer.sem_num = sem;
	sembuffer.sem_op = 1;
	sembuffer.sem_flg = SEM_UNDO;

	if(semop(semID, &sembuffer, 1) == -1){
		perror("Error in semop");
		exit(-1);
	}
}

int str2int(char *str){
	return strtol(str, 0, 10); 
}

float str2float(char *str){
	char *end;
	return strtof(str, &end);
}

