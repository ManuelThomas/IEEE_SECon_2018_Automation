#include "stdio.h"
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "setup.h"

int sockfd = -1;

void error(char *msg){
	perror(msg);
	//exit(0);
}

void setupComm(){ 
    int portno = 1333;      //Specifies the port to be used
    char serverIp[] = "192.168.0.100";    //Specifies the IP address of the server
    //char serverIp[] = "127.0.0.1";    //Specifies the IP address of the server
    struct sockaddr_in serv_addr;       //strucs that will contain information of the server 
    struct hostent *server;             
    char buffer[256];
    int data;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        error("ERROR opening socket");
        return;
    }

    if ((server = gethostbyname(serverIp)) == NULL){
        error("ERROR, no such host\n");
        sockfd = -1;
        return;
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR connecting");
        sockfd = -1;
        return;
    }
}

void sendStringData(char data[], int messageType){
    if (sockfd < 0)
        return;
	char buffer[256];
	bzero(buffer,256); // Clean the buffer

    switch(messageType){
		case COORDINATES:
			strncpy(buffer, "C: ", 3);
			strcat(buffer, data);
		break;
	}
	
    // Send data to the server
    if(write(sockfd, buffer, strlen(buffer)) < 0) error("ERROR writing to socket");
    // Clean the buffer and receive confirmation from the server
    bzero(buffer,256);
    if (read(sockfd,buffer,255)<0) error("ERROR reading from socket");
}

void sendData(float data, int messageType){

    if (sockfd < 0)
        return;

	char buffer[256];
	char floatBuffer[64];
	bzero(buffer,256); // Clean the buffer

    sprintf(floatBuffer,"%f",data);

    //Depending on the message type, place a character before the message for further interpretation on the server side.
	switch(messageType){
		case TIME:
			strncpy(buffer, "T: ", 3);
		break;
        case STAGES:
			strncpy(buffer, "S: ", 3);
        break;
        case ENCODER:
			strncpy(buffer, "E: ", 3);
        break;
        case PLANK:
			strncpy(buffer, "B: ", 3);
        break;
        case RESET:
			strncpy(buffer, "R: ", 3);
        break;
        case SCORE:
			strncpy(buffer, "P: ", 3);
        break;
	}

    strcat(buffer, floatBuffer);
    // Send data to the server

    if(write(sockfd, buffer, strlen(buffer)) < 0) error("ERROR writing to socket");

    // Clean the buffer and receive confirmation from the server
    bzero(buffer,256);

    if (read(sockfd,buffer,255)<0) error("ERROR reading from socket");
}