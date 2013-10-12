/*
* Matt Dumford - mdumford
*
* WORKER
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

int str2int(char *str);
float str2float(char *str);

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
	//int semID;
	//if(argc == 7)
		//semID = str2int(argv[6]);
	
	char workerPath[1024];
	getcwd(workerPath, sizeof(workerPath));
	strcat(workerPath, "/worker");

	//printf(ftok(workerPath, 'M'), msgID);
	int msgQ = msgID;
	if(msgQ == -1){
		perror("Error connecting to message queue");
		exit(-1);
	}

	struct message msg;
	msg.mtype = 1;
	msg.workerID = workerID;
	msg.sleepTime = sleepTime;
	//sprintf(msg.msg, "WorkerID: %i, sleepTime: %f.", workerID, sleepTime);

	if(msgsnd(msgQ, &msg, sizeof(struct message) - sizeof(long), 0) == -1){
		perror("Error sending message");
		exit(-1);
	}
	
	//===================== PART 3/4 ============================

	int *shm = shmat(shmID, (void *)0, 0);
	if(shm == (int *) -1){
		perror("Error attatching to shared memory ");
		exit(-1);
	}

	int i;
	int currentBuffer = workerID;
	for(i=0; i<nBuffers; i++){
		int j;

		for(j=0; j<2; j++){ //read twice
			int read = shm[currentBuffer];

			if(usleep(sleepTime*100000) == -1){
				perror("Error sleeping ");
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
					perror("Error sending message ");
					exit(-1);
				}
			}
	
			currentBuffer += workerID;
			if(currentBuffer >= nBuffers)
				currentBuffer -= nBuffers;
		}

		//write
		int read = shm[currentBuffer];

		if(usleep(sleepTime*100000) == -1){
			perror("Error sleeping ");
			exit(-1);
		}
		
		shm[currentBuffer] = read | (1<<(workerID -1));

		currentBuffer += workerID;
		if(currentBuffer >= nBuffers)
			currentBuffer -= nBuffers;

	}

	msg.mtype = 2;
	//sprintf(msg.msg, "Cleanup");
	msg.workerID = workerID;
	if(msgsnd(msgQ, &msg, sizeof(struct message), 0) == -1){
		perror("Error sending message ");
		exit(-1);
	}

	exit(0);
}

int str2int(char *str){
	return strtol(str, 0, 10); 
}

float str2float(char *str){
	char *end;
	return strtof(str, &end);
}

