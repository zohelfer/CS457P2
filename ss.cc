#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <string>

using namespace std;

#define MIN_PORT 1024
#define MAX_PORT 65535
#define MAX_LISTEN 5

void printUsage(){
	printf("Usage:\n'-h': Display this help message.\n'-p': Specifiy port number.\n'-s': Example: ./ss -p 3360");
}

void printError(string error){
	cerr << "Error: " << error << endl;
	exit(1);
}	

void checkValidPort(int portN){
	if(portN < MIN_PORT || portN > MAX_PORT){
		printError("Not a valid port number");
	}
}

void handleArgs(int argc, char **argv, int *portNum){
	int option;

	while((option = getopt(argc, argv, "sp:")) != -1){
		switch(option){
			case 'p': *portNum = atoi(optarg); break;
			default: printUsage(); exit(1);
		}
	}
	
	checkValidPort(*portNum);
}

int createSocket(int protocolFam, int type, int protocol){
	int mySocket = socket(protocolFam, type, protocol);
	if (mySocket < 0) printError("Could not create socket");
	return mySocket;
}

struct sockaddr_in createSockAddr(int family, int portNumber, int ipAddr){
	struct sockaddr_in thisAddr;

	thisAddr.sin_family = family;
	thisAddr.sin_addr.s_addr = htonl(ipAddr);
	thisAddr.sin_port = htons(portNumber);
	
	return thisAddr;
}

int main(int argc, char **argv){
	// Defaults
	int portNum = 0; //random port 0
	char hostName[128];
	socklen_t lenSockAddr = sizeof(struct sockaddr_in); //socklen_t is int

	// Port is specified
	if(argc == 2){
		char* portString = argv[1];
		portNum = atoi(portString);
		checkValidPort(portNum);
	}
	if(argc == 3){
		handleArgs(argc, argv, &portNum);
	}

	// Getting hostname
	gethostname(hostName, sizeof(hostName));

	// Create sockaddrs
	struct sockaddr_in servAddr;

	int servSocket = createSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // TCP socket
	servAddr = createSockAddr(AF_INET, portNum, INADDR_ANY);

	if(bind(servSocket, (struct sockaddr*)&servAddr, lenSockAddr) < 0){
		printError("Could not bind!");
	}

	if(listen(servSocket, MAX_LISTEN) != 0){
		printError("Could not listen.");
	}

	// Necessary for getting random port
	getsockname(servSocket, (struct sockaddr*) &servAddr, &lenSockAddr);
	printf("Waiting for a connection on HOST: %s PORT: %hu\n", hostName, ntohs(servAddr.sin_port));

}
