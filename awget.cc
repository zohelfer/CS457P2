#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <cstring>
#include <time.h>
#include "awget.h"

string FILENAME = "";

/*
Summary:
    Method goes to the last "/" of a url to create a substring of the filename.
    If there is no "/" the default filename is index.html
 */
string getFileName (string URL)
{
    string filename = "";
    if(URL.find("/") > (strlen(URL.c_str()) +1))
    {
        return "index.html";
    }
    filename = strrchr(URL.c_str(), '/') + 1;
    if (filename.compare("") == 0)
    {
        filename = "index.html";
    }
    //cout << filename << endl;
    return filename;
}

/*
Summary:
    Method opens a file with specified filename. The files is appended with each 1024 sized packet.
    Once the file is finished transferring the file is closed.
 */
void receiveFile(int sockID)
{
    FILE *recvFile = fopen(FILENAME.c_str(), "a");
    char receivedBuffer [1024];
    if (recvFile == NULL)
        perror ("Error!: Unable to open file");

    bzero(receivedBuffer, 1024);
    int recvSize = 0;
    while((recvSize = recv(sockID, receivedBuffer, 1024, 0)) > 0)
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
    printf("Ok received from client!\n");
    fclose(recvFile);
}

/*
Summary:
    Method sends a message to first SS with the size of the next message.
    The second message contains the list of IPs that the SS can chose from.
 */
void sendFile(int sockID, REQUEST requestMessage)
{
    printRequest(requestMessage);

    int sent = send(sockID, &requestMessage, sizeof(requestMessage), 0);
    if (sent == -1){
        cout << "Did not send dawg" << endl;
    }
    else { 
        cout << "Sent message" << endl;
    }
    struct sendSize
    {
        int messageSize;
    };

    string messageString = requestMessage.URLandIP;

    struct sendSize messageSendSize;
    messageSendSize.messageSize = strlen(messageString.c_str());
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

/*
Summary:
    Method reads the chainfile and parses the data.
    Data is stored in a REQUEST struct.
 */
vector<string> readChainFile (const char* filename, string URL)
{
    printf("chainlist is\n");
    
    string line = "";
   
    vector<string> collectionFile;
    collectionFile.push_back(URL);

    int lineCount = 0;

    ifstream ifile (filename);
    if (ifile.is_open())
    {
        while (!ifile.eof()){
            while (getline (ifile,line)) //goes through each line
            {
                if(lineCount == 0)
                {
                    collectionFile.push_back(line);
                    lineCount++;
                }
                else
                {   
                    int i=0;
                    while(line.at(i) != ' '){
                        i++;
                    }
                    string myIP = line.substr(0, i);
                    string myPort = line.substr(i, line.length());
                    cout << "<" << myIP << "," << myPort << ">" << endl;

                    string temp = myIP + "," + myPort + ";";
                    collectionFile.push_back(temp);
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

    return collectionFile;
}

/*
Summary:
    Method sets up a connection with a SS.
 */
void client(const char* ip, const char* portNum, REQUEST requestMessage)
{

    int sockID;
    struct sockaddr_in dest_addr;
    if ((sockID = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) >= 0)
    {
        printf("%s","Connecting to server... ");
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(atoi(portNum));
        dest_addr.sin_addr.s_addr =  inet_addr(ip);

        if(connect(sockID, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) >= 0)
        {
            printf("%s\n","Connected!" );
            sendFile(sockID, requestMessage);
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

vector<string> getIPandPort(string ipAndPort){
    int i=0;
    while(ipAndPort.at(i) != ',') i++;

    int end = i;
    int portLen = 0;
    while(ipAndPort.at(end) != ';'){
        end++;
        portLen++;
    } 

    vector<string> ipPort;

    ipPort.push_back(ipAndPort.substr(0,i));
    ipPort.push_back(ipAndPort.substr(i+1, portLen-1)); // Gets rid of ',' and ';'

    return ipPort;
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

    printf("Request: %s\n", URL.c_str());
    FILENAME = getFileName(URL);

    vector<string> fileInVec;
    fileInVec = readChainFile(chainFile.c_str(),URL);

    // Random index
    srand(time(NULL));
    int randIndex = (2 + rand() % (fileInVec.size() - 3)); // 2 <= Index <= Length

    string randRemoved = fileInVec.at(randIndex);
    fileInVec.erase(fileInVec.begin() + randIndex); // remove random IP

    // Getting IP and PORT to send to
    vector<string> ipAndPort;
    ipAndPort = getIPandPort(randRemoved);
    string IP = ipAndPort.at(0);
    string PORT = ipAndPort.at(1);

    cout << "next SS is <" + IP + "," + PORT + ">" <<  endl;
    cout << "waiting for file..." << endl;

    // IMPLEMENT CLIENT HERE
    string toSend;
    toSend = fileInVec.at(0) + "!" + fileInVec.at(1) + "!";
    
    for(unsigned int i=2; i < fileInVec.size(); i++) toSend += fileInVec.at(i);

    cout << "SEND: " << toSend;

    struct REQUEST requestMessage;
    requestMessage.URLandIP = toSend;
    client(IP.c_str(),PORT.c_str(),requestMessage);
}