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

int str2int(char *str);

int main(int argc, char **argv){
	if(argc < 6 || argc > 7){
		printf("Wrong number of arguments!");
		exit(-1);
	}
	
	int workerID = str2int(argv[1]);
	//int nBuffers = str2int(argv[2]);
	float sleepTime = str2int(argv[3]);
	//int msgID = str2int(argv[4]);
	//int shmID = str2int(argv[5]);
	//int semID;
	//if(argc == 7)
		//semID = str2int(argv[6]);
	
	char workerPath[1024];
	getcwd(workerPath, sizeof(workerPath));
	strcat(workerPath, "/worker");

	//printf(ftok(workerPath, 'M'), msgID);
	int msgQ = msgget(ftok(workerPath, 'M'), 00600);
	if(msgQ == -1){
		perror("Error connecting to message queue");
		exit(-1);
	}

	printf("Worker %i successfully  started.\n", workerID);

	struct message msg;
	msg.mtype = 5;
	sprintf(msg.msg, "WorkerID: %i, sleepTime: %f.", workerID, sleepTime);

	if(msgsnd(msgQ, &msg, sizeof(struct message) - sizeof(long), 0) == -1){
		perror("Error sending message");
		exit(-1);
	}
	
	printf("Worker %i finished.\n", workerID);
	exit(0);
}

int str2int(char *str){
        return strtol(str, 0, 10); 
}

