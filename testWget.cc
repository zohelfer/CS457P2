#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <cstring>

using namespace std;
int main (int argc, char* argv[]){

    char *website = argv[1];
    string systemCall ="";
    systemCall += "wget ";
    systemCall += website;
    systemCall += " -q";
    system(systemCall.c_str());

    char *fileName;

    fileName = strrchr(website, '/') + 1;

    remove(fileName);
}
