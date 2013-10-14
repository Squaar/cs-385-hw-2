struct message{
	long mtype;
	//char msg[256];
	int workerID;
	float sleepTime;
	int changedBuffer;
	int initVal;
	int newVal;
} message;

//mtypes:
//1-string message
//2-cleanup
//3-read error
