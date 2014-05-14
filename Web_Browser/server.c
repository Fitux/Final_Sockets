
#include "global.h"
#include "server.h"
#include "tcp.h"
#include "debug.h"

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

void clientProccess(const int clientSocket);

int startServer(const unsigned int port) {

	int serverSocket;
	int clientSocket;
	char clientIP[16];
	unsigned int clientPort;
	
	int pid;
	
	serverSocket = newTCPServerSocket4("0.0.0.0",port,5);
	if(serverSocket == -1) {
		error(errno,"Can't create Socket");
		return 0;
	}
	debug(4,"Server Socket Created");

	while(1) {
		clientSocket = waitConnection4(serverSocket,clientIP,&clientPort);
		debug(2,"Connected Client %s:%u",clientIP,clientPort);
		
		pid = fork();
		if(pid == -1) {
			debug(0,"Can't Fork!");
			close(clientSocket);
			continue;
		} else if(pid == 0 ) {
			// Soy el Hijo.
			clientProccess(clientSocket);
		} else if (pid > 0 ) {
			// Soy el Padre
			close(clientSocket);
		}
    }

	return 1;
	
}

char * itoa(int i) {
  char * res = malloc(8*sizeof(int));
  sprintf(res, "%d", i);
  return res;
}

void clientProccess(const int clientSocket) {
	
	char *buffer;
	char *firstLine, *host, *usr_agent, *content;		//REQUEST HEADERS
	char html[250];
	int file;
	int readBytes;
	int firstFlag;
	int tmp_size;
	
	char *type;
	char *request;
	char *protocol;
	char *pch;
	char *size;
	char *content_type;
	char *mime_type;

	debug(2,"Inicio del proceso del Cliente\n");
	
	type = calloc(255,1);
	request = calloc(255,1);
	protocol = calloc(255,1);

	buffer = calloc(255,1);

	firstLine = calloc(255,1);
	host = calloc(255,1);
	usr_agent = calloc(255,1);
	content = calloc(255,1);

	firstFlag = 1;
	while(readTCPLine4(clientSocket,buffer,254)>0) {
		debug(4,"%s",buffer);
		if(strcmp(buffer,"\r\n")==0) {
			break;
		}

		switch(firstFlag){				
			case 1: strcpy(firstLine, buffer);	//Catch request method from client
				firstFlag++;
				break;
			case 2: strcpy(host, buffer);		//Catch host
				firstFlag++;
				break;
			case 3: strcpy(usr_agent, buffer);	//Catch user agent from client
				firstFlag++;
				break;
			case 4: strcpy(content, buffer);	//Catch all content-type used
				firstFlag++;
				break;
			default: break;
		}
		bzero(buffer,255);
	}	
	
	//REQUEST HEADERS
	debug(4,"%s",firstLine);
	debug(4,"%s",host);
	debug(4,"%s",usr_agent);
	debug(4,"%s",content);

	// PROCESAR EL GET
	debug(4,"First Line %s",firstLine);
	pch = strtok (firstLine," /\r\n");
	while (pch != NULL)
	{
		if(strlen(type) == 0) 			//Catch type of request
			strcpy(type, pch);
		else if(strlen(request) == 0)		//Catch URI 
			strcpy(request, pch);	
		else if(strlen(protocol) == 0)		//Catch protocol
			strcpy(protocol, pch);
		pch = strtok(NULL, " /\r\n");

	}

	debug(4,"REQUEST %s", request);

	//RESPONDER CON 200 OK  SI EXISTE EL REQUEST
	if(strlen(request) == 0){
		sendStatus(clientSocket, "HTTP/1.1 200 OK");
	} else{
		file = open(request, O_RDONLY);			

		if(file == -1) {
			debug(4,"Error while open file %s", request);
			sendStatus(clientSocket, "HTTP/1.1 404 Not Found");
			strcpy(html, "<html><head><title>404 NOT FOUND</title></head><body>NO EXISTE!!!</body></html>\r\n");			
			sendTCPLine4(clientSocket, html,strlen(html));
			
			error(errno, "No se pudo abrir el archivo");
		}else{
			debug(4,"Preparing to send file %s", request);
			
			strcpy(html, "HTTP/1.1 200 OK");				//Send Content-type
			strcat(html, "\r\n");
			sendTCPLine4(clientSocket, html, strlen(html));	
			debug(4,"%s", html);

			strcpy(html, "Content-Type: ");				//Send Content-type
			strcat(html, "application/octet-stream");
			strcat(html, "\r\n");
			sendTCPLine4(clientSocket, html, strlen(html));	
			debug(4,"%s", html);

			strcpy(html, "Content-Disposition: ");				//Send Content-type
			strcat(html, "attachment");
			strcat(html, "\r\n");
			sendTCPLine4(clientSocket, html, strlen(html));	
			debug(4,"%s", html);	

			tmp_size = (int)fsize(request);			//Get size of file
			size = itoa (tmp_size);

			strcpy(html, "Content-Length: ");		//Send Content-lenght
			strcat(html, size);
			strcat(html, "\r\n");
			sendTCPLine4(clientSocket, html, strlen(html));
			debug(4,"%s", html);

			strcpy(html, "\r\n");	
			sendTCPLine4(clientSocket, html, strlen(html));
			debug(4,"%s", html);

			while((readBytes = read(file,buffer,255))>0) {		//Send file to client
				sendTCPLine4(clientSocket,buffer,readBytes);
			}
		}
	}
	
	//strcpy(html, "Content-Type: text/html\r\n");
	//sendTCPLine4(clientSocket, html, strlen(html));

	//strcpy(html, "Content-Lenght: 457838592\r\n");
	//sendTCPLine4(clientSocket, html,strlen(html));

	
	//CERRAMOS LA COMUNICACIÓN
	close(clientSocket);
	free(buffer);
	free(firstLine);
	
	return;
}
