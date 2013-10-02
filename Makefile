all: master worker

clean:
	rm {master,worker}

master: master.c
	gcc -Wall master.c -o master

worker: worker.c
	gcc  -Wall worker.c -o worker


