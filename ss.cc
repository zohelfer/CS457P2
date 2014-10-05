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
#include <pthread.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int numThreads = 0;
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

void* checkChainGang(void *passedArg){
	printf("New Thread ------------------------------------------\n");
	int messageIn = *((int*) passedArg);	
	int myTNum = numThreads;
	while(1){
		char newRM[200];
		recv(messageIn, &newRM, sizeof(newRM), 0);
		cout << myTNum <<" Recived: " << newRM << endl;
		char sendM[200];
		cout << myTNum << " Send: ";
		cin >> sendM; 
		int sentM = send(messageIn, &sendM, sizeof(sendM), 0);
		if (sentM < 0) printError("Could not send message!");	
	}
	printf("Exited Thread ------------------------------------------\n");
}

void handleArgs(int argc, char** argv, char** ipAddr, int* portNum){
	int option;
	while((option = getopt(argc, argv, "p:s:")) != -1){
		switch(option){
			case 's': *ipAddr = optarg; break;
			case 'p': *portNum = atoi(optarg); break;
			default: printUsage(); exit(1);
		}
	}

	struct sockaddr_in fakeAddr;
	if(!inet_aton(*ipAddr, &fakeAddr.sin_addr)){
		printError("Not a valid IP");
	}

	if( *portNum < 1024 || *portNum > 65535){
		printError("Not a valid port number");
	}

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
	if(argc > 3){
		//Client --- this code can go in wget
		char* ipAddr = NULL;
		int portNum = 0;

		handleArgs(argc, argv, &ipAddr, &portNum);

		if(ipAddr == NULL || portNum == 0){ printf("Arguments not specified correctly.\n"); printUsage(); exit(1); }
	
		//printf("You have selected \nIP:%s \nPort:%d\n", ipAddr, portNum);
		printf("Connecting to server...\n");

		int clientSocket;
		struct sockaddr_in clientAddr;

		//clientAddr = createSockAddr(AF_INET, portNum, inet_addr(ipAddr));
		clientAddr.sin_family = AF_INET;
		clientAddr.sin_port = htons(portNum);
		clientAddr.sin_addr.s_addr = inet_addr(ipAddr);

		clientSocket = createSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		if(connect(clientSocket, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) < 0){
			perror("Error: Could not connect to Server."); exit(1);
		}
		printf("Connected!\nConnected to a friend! You send first.\n");
		
		while(1){
			cout << "Send: " << endl;
			char sendMe[200];
			cin >> sendMe;
			int sendingM = send(clientSocket, &sendMe, sizeof(sendMe), 0);
			if (sendingM < 0) printError("Could not send message!");
			
			char recvM[200];
			int recivedM = recv(clientSocket, &recvM, sizeof(recvM), 0);
			cout << "Received: " << recvM << endl;
			if (recivedM < 0) printError("Could not receive message!");
			if (recivedM == 0) printError("Connection closed. Thank you for using chat.");
			
		}
	}

	// Getting hostname
	gethostname(hostName, sizeof(hostName));

	// Create sockaddrs
	struct sockaddr_in servAddr, cliAddr;

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

	//Create a thread array here
	std::vector<pthread_t> threads(10);

	//char *recievedM;
	int threadCount = 0;
	
	while(1){
		int messageIn = accept(servSocket, (struct sockaddr*)&cliAddr, &lenSockAddr);

		int th = pthread_create(&threads.at(threadCount), NULL, checkChainGang, (void*)(int*) &messageIn);
		if(th){
			printf("ERROR WITH PTHREAD\n");
		}
		printf("Done creating thread");
		threadCount++;
		numThreads++;
		/*
		if(messageIn < 0) printError("Could not accept.");
		printf("Mallocing!\n");
		recievedM = (char*) malloc(200 * sizeof(char));	
		printf("Done malloc\n");
		for(int i=0; i<200; i++){
			recievedM[i] = '\0';
		}
		cout << "About to recieve. "<< endl;
		int msgRecv = recv(messageIn, &recievedM, sizeof(recievedM), 0);
		cout << "RECIVED: " << recievedM << endl;

		if(msgRecv < 0) printError("Message not recived!");
		if(msgRecv == 0) printError("Connection closed. Goodbye.");
		cout << "going into thread!" << endl;
		*/
		
		//threads.push_back(tempThread);

		//std::thread newThread (checkChainGang, recievedM, messageIn);
		//newThread.join();
	}
	for(int i=0; i <= threadCount; i++ ){
		pthread_join(threads.at(threadCount), NULL);	
	}
	// Join all threads here
	
}
