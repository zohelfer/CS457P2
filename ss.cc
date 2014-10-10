/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

using namespace std;
//the thread function
void *connection_handler(void *);

char* getIP()
{
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

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    char* ip = getIP();
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = 0;

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(socket_desc, (struct sockaddr *)&sin, &len);
    printf("%s %s %s %d\n","Waiting for connection on",ip,"port", ntohs(sin.sin_port) );
    c = sizeof(struct sockaddr_in);


    //Accept and incoming connection
    c = sizeof(struct sockaddr_in);
    pthread_t thread_id;


    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}

string handShake (int friendID)
{
    struct sendSize
    {
        int mesSize;
    };
    string output;
    // while(true)
    // {
    int recvMesSize = 0;

    struct sendSize recvMessageSize;
    recvMesSize  = recv(friendID,&recvMessageSize,sizeof(recvMessageSize), 0);
    if (recvMesSize  > 0)
    {
        char temp_buff[recvMessageSize.mesSize];
        memset(temp_buff,'\0',strlen(temp_buff));
        int recvSize = 0;

        recvSize = recv(friendID, &temp_buff, sizeof(temp_buff), 0);
        if (recvSize > 0)
        {
            temp_buff[recvMessageSize.mesSize] = '\0';
            string tempSTR(temp_buff);
            output = tempSTR;
            //   break;
        }
    }
// }
    return output;
}

string parseAndRemove (int numberOfSS, string IPList, string& out)
{
    string output = "";
    srand(time(NULL));
    int choice = rand() % numberOfSS;
    int start_pos = 0;
    int end_pos = IPList.find(",");

    for (int i = 0; i < choice; i++)
    {
        start_pos = end_pos+1;
        end_pos = IPList.find(",",end_pos+1);
    }
    out = IPList.substr(start_pos,end_pos-start_pos);
    IPList.replace(start_pos,end_pos-start_pos+1,"");

    return IPList;

}

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
    cout << filename << endl;
    return filename;
}

void sendFile (int friendID, string website)
{
    string systemCall ="";
    systemCall += "wget ";
    systemCall += website;
    systemCall += " -q";
    system(systemCall.c_str());

    string fileName = getFileName(website);


    FILE *sendFile;
    sendFile = fopen (fileName.c_str() , "r");
    int segSize;
    char sendBuffer[1024];

    while((segSize = fread(sendBuffer, sizeof(char), 1024, sendFile)) > 0)
    {
        if(send(friendID, sendBuffer, segSize, 0) < 0)
        {
            //   fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", "FILE", errno);
            break;
        }
        bzero(sendBuffer, 1024);
    }
    printf("Ok File %s from Client was Sent!\n", "FILE");
    fclose (sendFile);
    remove(fileName.c_str());
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;

    string infoString = handShake(sock);
    int numberSS = atoi(infoString.substr(0,infoString.find(';')).c_str());

    if (numberSS > 0)
    {
        int pos = infoString.find(';');
        string nextIP = "";
        string parseStr = infoString.substr(pos+1, infoString.find(';',pos+1)-2);
        string newIPList = parseAndRemove(numberSS,parseStr,nextIP);

        string URL = infoString.substr(infoString.find(';',pos+1)+1, strlen(infoString.c_str()));

        numberSS--;

        string messageString = "";
        char intbuffer[2];
        sprintf(intbuffer,"%d",numberSS);
        string ConvertInt(intbuffer);
        messageString += ConvertInt;
        messageString += ";";
        messageString += newIPList;
        messageString += ";";
        messageString += URL;

        cout << messageString << endl;

        string ip = nextIP.substr(0,nextIP.find(' '));
        string portNum = nextIP.substr(nextIP.find(' ')+1,strlen(nextIP.c_str()));
        int sockID;
        struct sockaddr_in dest_addr;

        if ((sockID = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) >= 0)
        {

            printf("%s","Connecting to next SS... ");
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(atoi(portNum.c_str()));
            dest_addr.sin_addr.s_addr =  inet_addr(ip.c_str());

            if(connect(sockID, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) >= 0)
            {
                printf("%s\n","Connected!" );

                struct sendSize
                {
                    int mesSize;
                };
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

                char receivedBuffer [1024];
                int recvSize;
                while((recvSize = recv(sockID, receivedBuffer, 1024,0)) > 0)
                {
                    if (send(sock, receivedBuffer,1024,0) > 0)
                    {
                        bzero(receivedBuffer, 1024);
                    }
                }

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

    else
    {
        int pos = infoString.find(';');
        string URL = infoString.substr(infoString.find(';',pos+1)+1, strlen(infoString.c_str()));
        cout << "REQUEST " << URL << endl;
        sendFile(sock,URL);
    }

    return 0;
} 