#Makefile
#
# Instrucciones para compilar

OBJS = tcp.o server.o main.o
CC = gcc
CFLAGS = -Wall -O2

all: $(OBJS) webserver

webserver: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) webserver

push: 
	git add . 
	git commit -m 'Commit from make'
	git push

pull:
	git pull
