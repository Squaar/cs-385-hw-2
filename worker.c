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

int str2int(char *str);

int main(int argc, char **argv){
	if(argc < 6 || argc > 7){
		printf("Wrong number of arguments!");
		exit(-1);
	}
	
	int workerID = str2int(argv[1]);
	//int nBuffers = str2int(argv[2]);
	float sleepTime = str2int(argv[3]);
	int msgID = str2int(argv[4]); //msgget key
	//int shmID = str2int(argv[5]);
	//int semID;
	//if(argc == 7)
		//semID = str2int(argv[6]);

	int msgQ = msgget(msgID, 00660);
	if(msgQ == -1){
		perror("Error connecting to message queue: ");
		exit(-1);
	}

	printf("Worker %i successfully  started.\n", workerID);

	char msgBuffer[256] = "";
	sprintf(msgBuffer, "workerID: %i, sleepTime: %f.", workerID, sleepTime);

	struct message msg = {5, msgBuffer};
	if(msgsnd(msgQ, &msg, sizeof(struct message) - sizeof(long), 0) == -1){
		perror("Error sending message");
		exit(-1);
	}

	exit(0);
}

int str2int(char *str){
        return strtol(str, 0, 10); 
}

