#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <cstring>
#include <time.h>
#include <unistd.h>
#include "awget.h"

using namespace std;

string FILENAME = "";

struct request{
    int numberOfSS;
    string IPList;
    string URL;
};

void receiveFile(int sockID)
{
    FILE *recvFile = fopen(FILENAME.c_str(), "a");
    char receivedBuffer [1024];
    if (recvFile == NULL)
        perror ("Error!: Unable to open file");

    bzero(receivedBuffer, 1024);
    int recvSize = 0;
    while((recvSize = recv(sockID, receivedBuffer, 1024,0)) > 0)
    {
        int saveSize = fwrite(receivedBuffer, sizeof(char), recvSize, recvFile);
        if(saveSize < recvSize)
        {
            printf("%s\n","Error!: File write failed on server.");
        }
        bzero(receivedBuffer, 1024);
        if (recvSize == 0 || recvSize != 1024)
        {
            break;
        }
    }
    if(recvSize < 0)
    {

        printf("%s\n","Error!: Received size < 0.");
        exit(1);
    }
    //printf("Ok received from client!\n");
    fclose(recvFile);
}

void sendFileRequest(int sockID, request requestMessage)
{
    struct sendSize
    {
        int mesSize;
    };

    string messageString = "";
    char intbuffer[2];
    sprintf(intbuffer,"%d",requestMessage.numberOfSS);
    string ConvertInt(intbuffer);
    messageString += ConvertInt;
    messageString += ";";
    messageString += requestMessage.IPList;
    messageString += ";";
    messageString += requestMessage.URL;
    struct sendSize messageSendSize;
    messageSendSize.mesSize = strlen(messageString.c_str());
    if (send(sockID,&messageSendSize,sizeof(messageSendSize), 0) > 0)
    {
        if (send(sockID,messageString.c_str(),strlen(messageString.c_str()), 0) < 0)
        {
            cout << "Error!: Unable to send message text!" << endl;
        }
    }
    else
    {
        cout << "Error: Unable to send message size!" << endl;
    }
}

request readChainFile (const char* filename, string URL)
{
    cout << "chainlist is" << endl;
    struct request requestMessage;

    string line;
    string output = "";
    int entryCount = 0;
    int lineCount = 0;
    ifstream ifile (filename);
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
                    cout << "<" << line << ">" << endl;
                    output += ",";
                }
            }
            ifile.close();
        }
    }
    else
    {
        cout << "Error!: Unable to open input file" << endl;
        exit(1);
    }

    requestMessage.numberOfSS = entryCount;
    requestMessage.IPList = output;
    requestMessage.URL = URL;

    return requestMessage;
}

string parseAndRemove (request& requestMessage)
{
    string output = "";
    srand(time(NULL));
    int choice = rand() % requestMessage.numberOfSS;
    int start_pos = 0;
    int end_pos = requestMessage.IPList.find(",");

    for (int i = 0; i < choice; i++)
    {
        start_pos = end_pos+1;
        end_pos = requestMessage.IPList.find(",",end_pos+1);
    }

    output = requestMessage.IPList.substr(start_pos,end_pos-start_pos);
    requestMessage.IPList.replace(start_pos,end_pos-start_pos+1,"");
    requestMessage.numberOfSS = requestMessage.numberOfSS - 1;
    return output;

}

void client(const char* ip, const char* portNum, request requestMessage)
{

    int sockID;
    struct sockaddr_in dest_addr;
    if ((sockID = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) >= 0)
    {

        //printf("%s","Connecting to server... ");
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(atoi(portNum));
        dest_addr.sin_addr.s_addr =  inet_addr(ip);

        if(connect(sockID, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) >= 0)
        {
            //printf("%s\n","Connected!" );
            sendFileRequest(sockID,requestMessage);
            cout << "Received file " << FILENAME << endl;
            cout << "Goodbye!" << endl;
            receiveFile(sockID);
        }
        else
        {
            cout << "Error: Unable to connect" << endl;
        }
    }
    else
    {
        cout << "Error: Unable to create socket " << endl;
    }
}


int main (int argc, char* argv[]){
    string URL;
    string chainFile = "chainfile.txt";

    if (argc == 2)
    {
        string temp(argv[1]);
        URL = temp;
    }
    else if (argc == 3)
    {
        string tempURL(argv[1]);
        URL = tempURL;
        string tempFN(argv[2]);
        chainFile = tempFN;
    }
    else
    {
        cout << "Error!: Invalid number of inputs" << endl;
        exit(1);
    }


    FILENAME = getFileName(URL);
    cout << "Request: " << URL << endl;

    struct request requestMessage;
    string output = "";
    requestMessage = readChainFile(chainFile.c_str(),URL);

    output = parseAndRemove(requestMessage);
    string IP = "";
    string PORT = "";

    IP = output.substr(0,output.find(" "));
    PORT = output.substr(output.find(" ")+1,strlen(output.c_str())-(output.find(" ")+1));

    cout << "next SS is <" << IP << ", " << PORT << ">" << endl;
    cout << "waiting for file ..." << endl;
    client(IP.c_str(),PORT.c_str(),requestMessage);
    
    return 0;
}