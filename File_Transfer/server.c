#include "netlib/tcp.h"
#include "utils/debug.h"
#include "defaults.h"
#include "server.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>

void *attendClient(void *ptr);
off_t fsize(const char *filename);

int start_server(const char iface[], const unsigned short port, const unsigned short maxClients) 
{

	int serverSocket;
	int clientSocket;
	pthread_t thread;
	
	char clientIP[18];
	unsigned int clientPort;
	
	/*
	int file;
	
	int localerror;
	
	int readBytes = 0;
	int totalReadBytes = 0;	
	int totalWriteBytes = 0;
	int writeBytes = 0;
	char *buffer = NULL;
	*/
	
	serverSocket = newTCPServerSocket4(iface,port,maxClients);
	
	if(serverSocket == -1) 
	{
		return false;
	}
	
	while(1)
	{
		memset(clientIP,0,sizeof(clientIP));
		clientPort = 0;
			
		debug(5,"%s","Waiting for a Client...");
		clientSocket = waitConnection4(serverSocket,clientIP,&clientPort);
		debug(4,"Connected Client %s:%u",clientIP,clientPort);

		pthread_create(&thread, NULL, attendClient, (void *)&clientSocket);

		/*
		if((file = open(filename,O_RDONLY)) == -1) 
		{
			localerror = errno;
			fprintf(stderr,"Can't open filename (%s)",strerror(localerror));
			return false;
		}

		buffer = (char *) calloc(1,BUFFERSIZE);

		while((readBytes = read(file,buffer,BUFFERSIZE))>0) 
		{

			debug(5,"\tSe leyeron de archivo %u bytes",readBytes);		
			totalWriteBytes = 0;
			while(totalWriteBytes < readBytes) 
			{
				writeBytes = write(clientSocket,buffer+totalWriteBytes,readBytes-totalWriteBytes);
				debug(5,"\tSe escribieron a %s:%u %u bytes",clientIP,clientPort,readBytes);
				totalWriteBytes += writeBytes;
			}
			totalReadBytes += readBytes;
		}	

		debug(3,"\t Se leyeron un total de %i de bytes\n",totalReadBytes);
		
		free(buffer);	
		close(file);
		closeTCPSocket(clientSocket);
		debug(4,"Close connection (%s:%u)",clientIP,clientPort);
		*/
	}


	closeTCPSocket(serverSocket);	
	return true;
	
}

void *attendClient(void *ptr)
{
	int clientSocket;
	char *filename;
	int file;
	int localerror;

	char *command;
	char **data;
	char *buffer;

	int fileSize;

	clientSocket = *((int *)ptr);
	filename = (char *) calloc(1,255);

	buffer = (char *) calloc(1,BUFFERSIZE);
	data = (char **) calloc(1,255*sizeof(char*));
	command = (char *) calloc(1,255);

	//READ GET
	memset(data,0,255*sizeof(char*));
	memset(command,0,255);
	memset(buffer,0,BUFFERSIZE);

	recvfrom(clientSocket,buffer,BUFFERSIZE,0,NULL,0);
	debug(5,"Read: %s",buffer);
	command = getCommand(buffer,data);

	debug(4,"Comando: %s",command);

	if(strcmp(command,"GET")!=0)
	{
		debug(5,"Comando incorrecto");
		free(filename);
		free(command);
		free(data);
		free(buffer);		
		closeTCPSocket(clientSocket);
		return NULL;
	}

	strcpy(filename,data[0]);


	//OPEN FILE
	if((file = open(filename,O_RDONLY)) == -1) 
	{
		//SEND NOT FOUND
		debug(4,"No se abrio el archivo %s",filename);
		localerror = errno;
		fprintf(stderr,"Can't open filename (%s)",strerror(localerror));

		memset(buffer,0,BUFFERSIZE);
		strcpy(buffer,"NOT FOUND\r\n");

		sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
		debug(4,"Sent: %s", buffer);

		free(filename);
		free(command);
		free(data);
		free(buffer);		
		closeTCPSocket(clientSocket);
		return NULL;
	}
	else
	{
		//SEND OK
		debug(4,"Se abrio el archivo %s",filename);

		memset(buffer,0,BUFFERSIZE);
		strcpy(buffer,"OK\r\n");

		sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
		debug(4,"Sent: %s", buffer);
	}

	//READ OK
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
		free(filename);
		free(command);
		free(data);
		free(buffer);		
		closeTCPSocket(clientSocket);
		return NULL;
	}

	//SEND SIZE
	fileSize = fsize(filename);
	debug(5,"El archivo pesa %u ",fileSize);

	memset(buffer,0,BUFFERSIZE);
	snprintf(buffer, BUFFERSIZE, "SIZE:%i\r\n", fileSize);

	sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
	debug(4,"Sent: %s", buffer);

	//READ OK
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
		free(filename);
		free(command);
		free(data);
		free(buffer);		
		closeTCPSocket(clientSocket);
		return NULL;
	}

	//SEND FILE
	debug(4,"Sendind file: %s", filename);
	memset(buffer,0,BUFFERSIZE);
	
	while(read(file,buffer,BUFFERSIZE)>0) 
	{	
		sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
		debug(5,"Sent from file: %s",buffer);
		memset(buffer,0,BUFFERSIZE);	
	}	

	debug(4,"File Sent");
	close(file);

	//READ OK
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
		free(filename);
		free(command);
		free(data);
		free(buffer);		
		closeTCPSocket(clientSocket);
		return NULL;
	}

	//SEND BYE
	memset(buffer,0,BUFFERSIZE);
	strcpy(buffer,"BYE\r\n");

	sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
	debug(4,"Sent: %s", buffer);

	//READ BYE
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
		free(filename);
		free(command);
		free(data);
		free(buffer);		
		closeTCPSocket(clientSocket);
		return NULL;
	}

	free(filename);
	free(command);
	free(data);
	free(buffer);
	closeTCPSocket(clientSocket);
	return NULL;
}

off_t fsize(const char *filename) 
{
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}