/*
 *
 */

#include "../utils/debug.h"

#include <stdlib.h> 
#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int getNewTCPSocket(int addrType);

int buildAddr4(struct sockaddr_in *addr, const char *ip, const u_short port);
void sendStatus(const int socket, char *status_code);

int newTCPServerSocket4(const char *ip, const unsigned short port, const int q_size) {
	int socketFD;
	int status;
	int localerror;
	struct sockaddr_in addr;

	if(!buildAddr4(&addr,ip,port)) return -1;
	
	if((socketFD = getNewTCPSocket(PF_INET))==-1) return -1;
	
	status = bind(socketFD, (struct sockaddr*)&addr, sizeof(addr));
	if(status != 0) {
		localerror = errno;
		fprintf(stderr,"Error: Can't bind port %s:%i (%s)\n",ip,port,strerror(localerror));
		return -1;
	}
	
	status = listen(socketFD,q_size);
	if(status != 0) {
		localerror = errno;
		fprintf(stderr,"Error: Can't change socket mode to listen (%s)\n",strerror(localerror));
		return -1;
	}
	
	debug(4,"Socket on %s:%u created",ip,port);
	
	return socketFD;
}

int buildAddr4(struct sockaddr_in *addr, const char *ip, const u_short port) {
	
	int status;
	int localerror;

	memset(addr,0,sizeof(sockaddr_in));
	addr->sin_family = AF_INET;
	status = inet_pton(AF_INET,ip,&(addr->sin_addr.s_addr));
	if(status == 0) {
		fprintf(stderr,"Invalid IPv4 Address\n");
		return -1;
	} else if(status == -1) {
		localerror = errno;
		fprintf(stderr,"Error on IP Address (%s)\n",strerror(localerror));
		return -1;
	}
	
	addr->sin_port = htons(port);
	
	return -1;	
}

void closeTCPSocket(const int socketFD) {
	close(socketFD);
	debug(4,"Socket(%i) closed",socketFD);
	return;
}

int getNewTCPSocket(const int addrType) {
	int socketFD;
	int localerror;
	
	socketFD = socket(addrType,SOCK_STREAM,0);
	if(socketFD == -1) {
		localerror = errno;
		fprintf(stderr,"Can't create socket (%s)\n",strerror(localerror));
		return -1;
	}
	
	return socketFD;
}

int waitConnection4(int socket, char *clientIP, unsigned int *clientPort) {
	int client;
	struct sockaddr_in addrClient;
	socklen_t addrLen;
	char ip[INET_ADDRSTRLEN]={0};
	int localerror;
	
	addrLen = sizeof(addrClient);
	
	bzero(&addrClient, sizeof(addrClient));
	client = accept(socket, (struct sockaddr *)&addrClient,&addrLen);
	if(client == -1) {
		localerror = errno;
		fprintf(stderr,"Can't retrive client Socket (%s)\n",strerror(localerror));
		return -1;
	}
	
	if(clientIP!=NULL) {
		inet_ntop(AF_INET,&(addrClient.sin_addr),ip,INET_ADDRSTRLEN);
		strcpy(clientIP,ip);		
	}
	
	if(clientPort!=NULL) {
		*clientPort = ntohs(addrClient.sin_port);
	}
	
	return client;
}

int newTCPClientSocket4(const char *ip, const u_short port) {
	int clientSocket;
	int status;
	int localerror;
	struct sockaddr_in addr;
	
	if(!buildAddr4(&addr,ip,port)) return -1;
	
	if((clientSocket = getNewTCPSocket(PF_INET))==-1) return -1;
	
	status = connect(clientSocket, (struct sockaddr*)&addr, sizeof(addr));
	if(status == -1) {
		localerror = errno;
		fprintf(stderr,"Can't connect to %s:%i (%s)",ip,port,strerror(localerror));
		return -1;
	}
	
	debug(3,"Connected to %s:%i",ip,port);
	
	return clientSocket;
}

int readTCPLine4(const int socket, char *buffer, const unsigned int maxRead ) {
	char *ptr;
	int byte;
	unsigned int readBytes;
	
	ptr = buffer;
	readBytes = 0;
	
	while((byte = read(socket,ptr,1)) > 0) {
		if(*ptr == '\n' || *ptr == '\0') {
			break;
		}
		
		if(readBytes == maxRead) {
			break;
		}
		
		ptr++;
		readBytes++;
	}
	
	return readBytes;
}

int sendTCPLine4(const int socket, char *buffer, const unsigned int size ) {

	int sentBytes = 0;
	int writeBytes = 0;
	
	while((writeBytes = write(socket,buffer+sentBytes,size-sentBytes)) > 0) {
		sentBytes += writeBytes;
	}
	
	debug(6,"Sent %i Bytes\n",sentBytes);
	
	return sentBytes;
}

void sendStatus(const int socket, char *status_code){
	char html[250];
	
	strcpy(html, status_code);
	strcat(html, "\r\n");
	sendTCPLine4(socket, html, strlen(html));
	
	strcpy(html, "\r\n");
	sendTCPLine4(socket, html,strlen(html));
	return;
}

char *getCommand(char *buffer, char **args) 
{
    char *pch;
    char *command;
    int cont=0;

    command = (char *)calloc (255,1);

    pch = strtok (buffer," :\r\n"); 

    while (pch != NULL)
	{
		if(strlen(command) == 0)
		{
			strcpy(command, pch);
		}
		else if(args[cont] == NULL || strlen(args[cont]) == 0)
		{	
			if(args[cont] == NULL)
				args[cont] = (char *)calloc (255,1);
			strcpy(args[cont], pch);
			cont++;
		}	
	
		pch = strtok(NULL, " :\r\n");
	}

    return command; 
}
