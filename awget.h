#ifndef _INCL_AWGET
#define _INCL_AWGET

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

// Expecting URLandIP as
// URL!#IP!IP1,port1;IP2,port2
// Ex. URL!2!129.68.0.1,4231;129.23.1.2,2314
typedef struct REQUEST{
	string URLandIP;
} REQUEST;

//void printRequest(struct REQUEST);

void printRequest(struct REQUEST req){
    cout << "In REQUEST: " << req.URLandIP << endl;
}

string getFileName (string);

/*
Returns vector with following elements
[0] = URL -- "http..."
[1] = IP Count -- "3" 
[2..N] = IP and PORT combination -- "IP,Port;""
*/
vector<string> readChainFile(const char*, string);

#endif