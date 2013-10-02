all: master worker

clean:
	rm {master,worker}

master: master.c message.h
	gcc -Wall master.c -o master

worker: worker.c message.h
	gcc  -Wall worker.c -o worker


