struct message{
	long mtype;
	char msg[256];
	int workerID;
} message;

//mtypes:
//1-string message
//2-cleanup
