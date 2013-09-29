/*
* Matt Dumford 
* mdumfo2@uic.edu
* 
* MASTER
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

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
	int sleepMin = str2int(argv[3]);
	int sleepMax = str2int(argv[4]);
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

	pid_t pid = fork();

	if(pid < 0){
		printf("Error forking!\n");
		exit(-1);
	}
	if(pid ==0){ //CHILD PROCESS
		close(pipe1[WRITE]);
		close(pipe2[READ]);

		dup2(pipe1[READ], STDIN);
		dup2(pipe2[WRITE], STDOUT);

		char *sort[2] = {"sort", "-nr"};
		//execvp(sort[0], sort);

		//exec shouldn't return
		//printf("Error sorting!\n");
		//exit(-1);
	}
	else{ //PARENT PROCESS
		close(pipe1[READ]);
		close(pipe2[WRITE]);

		if(randSeed == 0)
			srand(time(NULL));
		else
			srand(randSeed);

		printf("in parent\n");
		int i;
		for(i=0; i<nWorkers; i++){
			int random = rand() % (sleepMax-sleepMin) + sleepMin;
			printf("%i\n", random);
			//sprintf(pipe1[WRITE], "%i\n", rand() % (sleepMax-sleepMin) + sleepMin);
		}
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
