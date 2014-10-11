#ifndef _INCL_AWGET
#define _INCL_AWGET

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <vector>

using namespace std;

// ------------------------------ Constants

#define MIN_PORT 1024
#define MAX_PORT 65535
#define MAX_LISTEN 12

// ------------------------------ Structs

struct sendSize{
    int mesSize;
};

// ------------------------------ Function declarations

void *connection_handler(void *); //Thread func

// ------------------------------ Helper functions
void printError(string error){
	cerr << "Error: " << error << endl; exit(1);
}	

void printUsage(){
	printf("Usage:\n'-h': Display this help message.\n'-p': Specifiy port number.\n'-s': Example: ./ss -p 3360");
}

string getFileName (string URL){
    string filename = "";
    if(URL.find("/") > (strlen(URL.c_str()) +1)){
        return "index.html";
    }
    filename = strrchr(URL.c_str(), '/') + 1;
    if (filename.compare("") == 0){
        filename = "index.html";
    }
    return filename;
}

char* getIP(){
    struct ifaddrs *ifaTemp, *ifa;
    struct sockaddr_in *temp;
    char *addr;

    getifaddrs (&ifaTemp);
    for (ifa = ifaTemp; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET && strcmp(ifa->ifa_name, "lo") != 0) {
            temp = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(temp->sin_addr);
            break;
        }
    }

    freeifaddrs(ifaTemp);
    return addr;
}

void checkValidPort(int portN){
    if(portN < MIN_PORT || portN > MAX_PORT) printError("Not a valid port number");
}

void handleArgs(int argc, char **argv, int *portNum){
    int option;
    while((option = getopt(argc, argv, "p:")) != -1){
        switch(option){
            case 'p': *portNum = atoi(optarg); break;
            default: printUsage(); exit(1);
        }
    }
    checkValidPort(*portNum);
}

#endif