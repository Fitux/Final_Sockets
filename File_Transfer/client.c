#include "netlib/tcp.h"
#include "utils/debug.h"
#include "defaults.h"
#include "client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/time.h>

unsigned long currentTimeMillis() ;

int start_client(const char *ip_dst, const unsigned int port, const char *filename) 
{

	int clientSocket;
	int file;
	
	int localerror;

	char *command;
	char **data;
	char *buffer;

	int filesize, readData;

	clientSocket = newTCPClientSocket4(ip_dst,port);

	buffer = (char *) calloc(1,BUFFERSIZE);
	data = (char **) calloc(1,255*sizeof(char*));
	command = (char *) calloc(1,255);

	//SEND GET
	memset(buffer,0,BUFFERSIZE);
	strcpy(buffer,"GET:");
	strcat(buffer,filename);
	strcat(buffer,"\r\n");

	sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
	debug(4,"Sent: %s", buffer);

	//Read OK or NOTFOUND
	memset(data,0,255*sizeof(char*));
	memset(command,0,255);
	memset(buffer,0,BUFFERSIZE);

	recvfrom(clientSocket,buffer,BUFFERSIZE,0,NULL,0);
	debug(5,"Read: %s",buffer);
	command = getCommand(buffer,data);


	debug(4,"Comando: %s",command);

	if(strcmp(command,"OK")!=0)
	{
		debug(5,"Comando incorrecto");
		free(command);
		free(data);
		free(buffer);		
		closeTCPSocket(clientSocket);
		return false;
	}

	//SEND OK
	memset(buffer,0,BUFFERSIZE);
	strcpy(buffer,"OK");
	strcat(buffer,"\r\n");

	sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
	debug(4,"Sent: %s", buffer);

	//Read Size
	memset(data,0,255*sizeof(char*));
	memset(command,0,255);
	memset(buffer,0,BUFFERSIZE);

	recvfrom(clientSocket,buffer,BUFFERSIZE,0,NULL,0);
	debug(5,"Read: %s",buffer);
	command = getCommand(buffer,data);

	debug(4,"Comando: %s",command);

	if(strcmp(command,"SIZE")!=0)
	{
		debug(5,"Comando incorrecto");
		free(command);
		free(data);
		free(buffer);		
		closeTCPSocket(clientSocket);
		return false;
	}

	filesize = atoi(data[0]);

	//SEND OK
	memset(buffer,0,BUFFERSIZE);
	strcpy(buffer,"OK");
	strcat(buffer,"\r\n");

	sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
	debug(4,"Sent: %s", buffer);
	
	//OPEN FILE
	if((file = open(filename+1,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1) 
	{
		debug(5,"Error creating file: %s", filename);
		localerror = errno;
		fprintf(stderr,"Can't open file for write (%s)\n",strerror(localerror));
		return false;
	}	

	readData = 0;
	debug(4,"Writing in file: %s", filename);

	while((readData += recvfrom(clientSocket,buffer,BUFFERSIZE,0,NULL,0))>0) 
	{	
		write(file,buffer,readData);
		debug(5,"Save to file: %s",buffer);	
		memset(buffer,0,BUFFERSIZE);

		if(readData>=filesize)
			break;
	}

	close(file);

	//SEND OK
	memset(buffer,0,BUFFERSIZE);
	strcpy(buffer,"OK");
	strcat(buffer,"\r\n");

	sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
	debug(4,"Sent: %s", buffer);

	//Read BYE
	memset(data,0,255*sizeof(char*));
	memset(command,0,255);
	memset(buffer,0,BUFFERSIZE);

	recvfrom(clientSocket,buffer,BUFFERSIZE,0,NULL,0);
	debug(5,"Read: %s",buffer);
	command = getCommand(buffer,data);

	debug(4,"Comando: %s",command);

	if(strcmp(command,"BYE")!=0)
	{
		debug(5,"Comando incorrecto");
		free(command);
		free(data);
		free(buffer);		
		closeTCPSocket(clientSocket);
		return false;
	}

	//SEND BYE
	memset(buffer,0,BUFFERSIZE);
	strcpy(buffer,"BYE");
	strcat(buffer,"\r\n");

	sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
	debug(4,"Sent: %s", buffer);

	free(command);
	free(data);
	free(buffer);	
	closeTCPSocket(clientSocket);
	
	return true;
}

unsigned long currentTimeMillis() 
{
        unsigned long t;
        struct timeval tv;
        gettimeofday(&tv, 0);
        t = (tv.tv_sec*1000)+tv.tv_usec;
        return t;
}