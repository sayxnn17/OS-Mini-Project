CC = gcc
CFLAGS = -Wall -pthread

all: master worker

master: master.c common.h
	$(CC) $(CFLAGS) master.c -o master

worker: worker.c common.h
	$(CC) $(CFLAGS) worker.c -o worker

clean:
	rm -f master worker /tmp/payload_task /tmp/output.txt /tmp/final_output.txt

run-worker:
	./worker

run-master:
	@echo "Usage: make run-master MYIP=<ip> W1=<ip> W2=<ip> FILE=<file.c>"
	./master $(MYIP) $(W1) $(W2) $(FILE)
