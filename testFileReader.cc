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

using namespace std;



int main (int argc, char* argv[]){
    string line;
    string output = "";
    int entryCount = 0;
    int lineCount = 0;
    ifstream ifile (argv[1]);
    if (ifile.is_open())
    {
        while (!ifile.eof()){
            while (getline (ifile,line)) //goes through each line
            {
                if(lineCount == 0)
                {
                    entryCount = atoi(line.c_str());
                    lineCount++;
                }
                else
                {
                    output += line;
                    output += ",";
                }
            }
            cout << entryCount << endl;
            cout << output << endl;
            ifile.close();
        }
    }
    else
    {
        cout << "Error!: Unable to open input file" << endl;
        exit(1);
    }

}