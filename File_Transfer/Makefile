#Makefile
#
# Instrucciones para compilar

NAME = ft
OBJS = main.o file_transfer.o server.o client.o netlib/tcp.o 

CC = g++
CFLAGS = -Wall -O2 -pthread

all: $(OBJS) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(NAME)
	rm -rf html

doc:
	doxygen
	


