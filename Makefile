all: master worker

clean:
	rm {master,worker}

master:
	gcc -Wall master.c -o master

worker:
	gcc  -Wall worker.c -o worker


