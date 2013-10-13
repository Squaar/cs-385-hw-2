/*
* Matt Dumford 
* mdumfo2@uic.edu
* 
* MASTER
*/

#include "message.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define BOOL int
#define TRUE 1
#define FALSE 0
#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1

union semun{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
} arg;

int isPrime(int n);
int str2int(char *str);

int main(int argc, char** argv){
	printf("Matt Dumford - mdumfo2\n");

	if(argc < 5 || argc > 7){
		printf("Invalid arguments!\n");
		exit(-1);
	}
	
	int nBuffers = str2int(argv[1]);
	int nWorkers = str2int(argv[2]);
	float sleepMin = str2int(argv[3]);
	float sleepMax = str2int(argv[4]);
	int randSeed = 0;
	BOOL lock = FALSE;
	
	if(!isPrime(nBuffers) || nBuffers > 32){
		printf("nBuffers must be prime and < 32!\n");
		exit(-1);
	}
	
	if(nWorkers > nBuffers){
		printf("nWorkers must be less than nBuffers!\n");
		exit(-1);
	}

	if(sleepMin<0 || sleepMax<0 || sleepMin > sleepMax){
		printf("sleepMin and sleepMax must be greater than 0, \n"\
				"and sleepMax must be greater than sleepMin!\n");
		exit(-1);
	}
	else if(argc == 7){
		if(!strcmp(argv[6], "-lock") || !strcmp(argv[6], "lock"));
			lock = TRUE;
		randSeed = str2int(argv[5]);
	}
	else if(argc == 6){
		if(!strcmp(argv[5], "-lock") || !strcmp(argv[5], "lock"))
			lock = TRUE;
		else if(!strcmp(argv[5], "-nolock") || !strcmp(argv[5], "nolock"))
			lock = FALSE;
		else
			randSeed = str2int(argv[5]);
	}

	//============================ DONE WITH SETUP ==============================
	
	int	pipe1[2]; //list of numbers from parent to child
	int pipe2[2]; //list of sorted numbers from child to parent

	if(pipe(pipe1) || pipe(pipe2)){
		perror("Error creating pipes");
		exit(-1);
	}

	pid_t pid = fork();

	if(pid < 0){
		perror("Error forking");
		exit(-1);
	}
	if(pid == 0){ //CHILD PROCESS
		close(pipe1[WRITE]);
		close(pipe2[READ]);

		dup2(pipe1[READ], STDIN);
		dup2(pipe2[WRITE], STDOUT);

		close(pipe1[READ]);
		close(pipe2[WRITE]);

		execlp("sort", "sort", "-gr", NULL);
   
		//exec shouldn't return
		perror("Error sorting");
		exit(-1);
	}
	else{ //PARENT PROCESS

		close(pipe1[READ]);
		close(pipe2[WRITE]);
	
		if(randSeed == 0)
			srand(time(NULL));
		else
			srand(randSeed);
	
		char outBuffer[256] = "";
	
		int i;		
		for(i=0; i<nWorkers; i++){
			float random = (rand()/(float) RAND_MAX) * (sleepMax-sleepMin) + sleepMin;
			if(i != nWorkers-1)
				sprintf(outBuffer + strlen(outBuffer), "%f\n", random);
			else
				sprintf(outBuffer + strlen(outBuffer), "%f", random);
		}
	
		printf("%s\n", outBuffer);
		write(pipe1[WRITE], outBuffer, strlen(outBuffer));
	
		close(pipe1[WRITE]);
		
		int *status = 0;
		pid_t pid2 = wait(status);
	
		//if child didn't exit normally
		if(pid2 == -1 || !WIFEXITED(status) || WEXITSTATUS(status)){
			perror("Error in child");
			exit(-1);
		}
		
		size_t nbytes = strlen(outBuffer);
		char inBuffer[nbytes];
		ssize_t readBytes = read(pipe2[READ], inBuffer, nbytes);
	
		if(readBytes == -1){
			printf("Error reading from pipe!\n");
			exit(-1);
		}
	
		close(pipe2[READ]);
	
		inBuffer[strlen(outBuffer)] = '\0';
	
		printf("========\n%s\n", inBuffer);

		int numToks = 0;
		char *sleepTimes[nWorkers];
		char *tok = strtok(inBuffer, "\n");

		while(tok != NULL){
			sleepTimes[numToks] = tok;
			tok = strtok(NULL, "\n");
			numToks++;
		}

		sleepTimes[numToks] = NULL;

		char workerPath[1024];
		getcwd(workerPath, sizeof(workerPath));
		strcat(workerPath, "/worker");

		//=============================== PART 3 ==============================

		int shmid = shmget(ftok(workerPath, 'N'), sizeof(int[nBuffers]), 00644|IPC_CREAT);
		if(shmid == -1){
			perror("Error creating shared memory ");
			exit(-1);
		}

		int *shm = shmat(shmid, (void *)0, 0);
		if(shm == (int *) -1){
			perror("Error attatching to shared memory ");
			exit(-1);
		}

		printf("\nSuccessfully connected to shared memory.\n");

		memset(shm, 0, nWorkers*sizeof(shm[0]));

		//=============================== PART 5 ===============================
		int semid;
		if(lock){
			if((semid = semget(ftok(workerPath, 'O'), nBuffers, 00644|IPC_CREAT)) == -1){
				perror("Error creating semaphores ");
				exit(-1);
			}

			arg.val = 1;
			
			for(i=0; i<nBuffers; i++){
				if(semctl(semid, i, SETVAL, arg) == -1){
					perror("Error initializing semaphore ");
					exit(-1);
				}
			}
		}

		//=============================== PART 2 ===============================
		
		

		int msgQ = msgget(ftok(workerPath, 'M'), IPC_CREAT | 00600);
		if(msgQ == -1){
			perror("msgget failed: ");
			exit(-1);
		}
		
		for(i=0; i<nWorkers; i++){ //fork off nWorkers workers
			pid = fork();
			if(pid < 0){
				perror("Error forking");
				exit(-1);
			}
			if(pid == 0){ //CHILD PROCESS
				char workerID[10];
				sprintf(workerID, "%i", i+1);

				char numBuffers[10];
				sprintf(numBuffers, "%i", nBuffers);

				char msgQID[10];
				sprintf(msgQID, "%i", msgQ);

				char shmID[10];
				sprintf(shmID, "%i", shmid);

				if(lock){
					char semID[10];
					sprintf(semID, "%i", semid);
					execlp(workerPath, "worker", workerID, numBuffers, sleepTimes[i], msgQID, shmID, semID, NULL);
				}
				else
					execlp(workerPath, "worker", workerID, numBuffers, sleepTimes[i], msgQID, shmID, NULL);

				perror("Error in worker");
				exit(-1);
			}
		}
		
		int unfinishedWorkers = nWorkers;
		while(unfinishedWorkers){ //read nWorkers messages
			struct message msg;
			//msg.msg = malloc(sizeof(struct message)-sizeof(long));
			if(msgrcv(msgQ, &msg, sizeof(struct message), 0, 0) == -1){
				perror("Error recieving message");
				exit(-1);
			}
			
			if(msg.mtype == 1) //startup message
				printf("Startup message recieved from worker %i with sleep time %i.\n", msg.workerID, msg.sleepTime);
			else if(msg.mtype ==2){ //cleanup message
				int *status = 0;
				pid_t pid2 = wait(status);
				printf("Recieved cleanup message from worker %i\n", msg.workerID);
				unfinishedWorkers--;
        	
				//if child didn't exit normally
				if(pid2 == -1 || !WIFEXITED(status) || WEXITSTATUS(status)){
           	 		perror("Error in worker");
					exit(-1);
				}
			}
			else if(msg.mtype == 3){ //read error
				printf("Read error message recieved: WorkerID: %i, changed buffer: %i, initial value: %i, new value: %i\n",
							   	msg.workerID, msg.changedBuffer, msg.initVal, msg.newVal);
			}
		}
		
		int expected = (1<<nWorkers)-1;
		printf("\nExpected value for buffers: %i\n", expected);
		printf("All workers accounted for, contents of shared memory: \n");
		for(i=0; i<nBuffers; i++)
			printf("%i\n", shm[i]);

		//check which bits are missing
		for(i=0; i<nBuffers; i++){
			int missing = shm[i] ^ expected;
			int j;
			for(j=0; j<sizeof(int)*8; j++){
				if(missing & 1<<j){
					printf("Buffer %i is missing bit %i!\n", i, j+1);
				}
			}
		}

		//remove message queue
		if(msgctl(msgQ, IPC_RMID, NULL)){
			perror("Error removing message queue");
			exit(-1);
		}

		if(shmdt(shm) == -1){
			perror("Error disconnecting from shared memory ");
			exit(-1);
		}
		if(shmctl(shmid, IPC_RMID, NULL) == -1){
			perror("Error removing shared memory ");
			exit(-1);
		}
		if(lock){
			if(semctl(semid, 0, IPC_RMID) == -1){
				perror("Error removing semaphores ");
				exit(-1);
			}
		}

	} //END PARENT
	exit(0);	
}

int str2int(char *str){
	return strtol(str, 0, 10); 
}

BOOL isPrime(int n){
	if(n < 2)
		return FALSE;
	int i;
	for(i=2; i*i<n; i++)
		if(n % i == 0)
			return FALSE;
	return TRUE;
}
