/*
* Matt Dumford 
* mdumfo2@uic.edu
* 
* MASTER
*/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BOOL int
#define TRUE 1
#define FALSE 0
#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1

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

	if(argc == 7){
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
		perror("Error creating pipes: ");
		exit(-1);
	}

	pid_t pid = fork();

	if(pid < 0){
		perror("Error forking: ");
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
		perror("Error sorting: ");
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
			perror("Error in child: ");
			exit(-1);
		}
		
		//READ FROM SORTED NUMBERS HERE
		size_t nbytes = strlen(outBuffer);
		char inBuffer[nbytes];
		ssize_t readBytes = read(pipe2[READ], inBuffer, nbytes);

		if(readBytes == -1){
			printf("Error reading from pipe!");
			exit(-1);
		}

		close(pipe2[READ]);

		printf("========\n");
		printf("%s\n", inBuffer);
	}


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
