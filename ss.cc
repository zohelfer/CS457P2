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

void printUsage(){
	printf("Usage:\n'-h': Display this help message.\n'-p': Specifiy port number.\n'-s': Example: ./ss -p 3360");
}

void printError(string error){
	cerr << "Error: " << error << endl;
	exit(1);
}	

int checkValidPort(int portN){
	if(portN < 1024 || portN > 65535){
		printError("Not a valid port number");
	}
	return 1;
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

int main(int argc, char **argv){
	int portNum = 0;
	//Port is specified
	if(argc == 2){
		char* portString = argv[1];
		portNum = atoi(portString);
		checkValidPort(portNum);
	}
	if(argc == 3){
		handleArgs(argc, argv, &portNum);
	}

	printf("Listening on : %d\n", portNum);

}
