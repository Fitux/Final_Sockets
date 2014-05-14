/**
  @file tcp.h
  @brief Funciones generales para trabajar con un Socket de tipo TCP

  @author Alvaro Parres
  @date Feb/2013

*/

#ifndef TCP_H
#define TCP_H

int newTCPServerSocket4(const char *ip, const unsigned short port, const int q_size);
void closeTCPSocket(const int socketFD);
int waitConnection4(int socket, char *clientIP, unsigned int *clientPort);
int newTCPClientSocket4(const char *ip, const unsigned short port);
int readTCPLine4(const int socket, char *buffer, const unsigned int maxRead );
int sendTCPLine4(const int socket, char *buffer, const unsigned int size );

#endif
