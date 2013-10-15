Matt Dumford - mdumford
mdumfo2@uic.edu

CS 385 - Homework 2 - Inter-Process Communications and Synchronization

master
	./master nBuffers nWorkers sleepMin sleepMax [ randSeed ] [ -lock | -nolock ]
	Runs the main program. Arguments:
	nBuffers: The number of buffers that will be created in the shared memory
	nWorkers: The number of workers that will be forked off and executed by the master
	sleepMin: Minimum number of seconds that workers will sleep in reads and writes
	sleepMax: Maximum number of seconds that workers will sleep in reads and writes
	randSeed: Optional - seed for the random number generator for the sleep times
	lock: -lock: use semaphores. -nolock: don't use semaphores. -nolock is the default.

	Output: The master program will first print the randomly generated sleep times as they 
	are generated, and then it will print them again in sorted order.
	It then prints all of the messages that it recieves from the workers, including start up messages,
	read error messages, and clean up messages. After all of the workers are finished, the master
	prints the contents of the shared memory and lists what bits are missing and from where.


worker
	The worker program is not meant to be run independantly of the master. It will not work 
	if it is run on its own.


make all
	makes master and worker


make master
	makes master only


make worker
	makes worker only


make clean
	deletes master and worker executables


