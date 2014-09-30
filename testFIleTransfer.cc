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

// Modified Source :)
// http://stackoverflow.com/questions/15627492/c-send-file-and-text-via-socket
// + our old chat program

/*

NOTES:
This runs exactly like our old chat program.
The only different is it transfers files.
To change the file it uses just run down to the fopen commands.
Let me know if you have any questions.
I am sure it will need some cleaning up but I think this program is almost done.
:)

*/
using namespace std;

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

void client(char* ip, char* portNum)
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
            printf("%s\n","Connected to a friend! You send the file." );


            //true = send message, false = receive messages

            FILE *sendFile;
            sendFile = fopen ("test.jpeg" , "r");
            int segSize;
            char sendBuffer[1024];

            while((segSize = fread(sendBuffer, sizeof(char), 1024, sendFile)) > 0)
            {
                if(send(sockID, sendBuffer, segSize, 0) < 0)
                {
                    //   fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", "FILE", errno);
                    break;
                }
                bzero(sendBuffer, 1024);
            }
            printf("Ok File %s from Client was Sent!\n", "FILE");
            fclose (sendFile);

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

void server()
{
    int servSock;
    unsigned int sin_size;
    int friendID;


    struct sockaddr_in ServAddr;
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_port = 0;
    struct sockaddr_in their_addr;

    if ((servSock= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) >= 0)
    {
        char* ip = getIP();
        ServAddr.sin_addr.s_addr = inet_addr(ip);

        printf("%s\n","Welome to Chat");
        if (bind(servSock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) >= 0)
        {
            if (listen(servSock, 5) >= 0)
            {
                struct sockaddr_in sin;
                socklen_t len = sizeof(sin);
                getsockname(servSock, (struct sockaddr *)&sin, &len);
                printf("%s %s %s %d\n","Waiting for connection on",ip,"port", ntohs(sin.sin_port) );

                sin_size = sizeof(struct sockaddr_in);
                friendID = accept(servSock,(struct sockaddr *) &their_addr, &sin_size);

                if (friendID >= 0)
                {
                    //true = send message, false = receive messages
                    cout << "Found a friend! You receive the file." << endl;

                    FILE *recvFile = fopen("NewPIC.jpeg", "a");
                    char receivedBuffer [1024];
                    if (recvFile == NULL)
                        perror ("Error opening file");

                    bzero(receivedBuffer, 1024);
                    int recvSize = 0;
                    while((recvSize = recv(friendID, receivedBuffer, 1024, 0)) > 0)
                    {
                        int saveSize = fwrite(receivedBuffer, sizeof(char), recvSize, recvFile);
                        if(saveSize < recvSize)
                        {
                            //error("File write failed on server.\n");
                        }
                        bzero(receivedBuffer, 1024);
                        if (recvSize == 0 || recvSize != 1024)
                        {
                            break;
                        }
                    }
                    if(recvSize < 0)
                    {

                        //fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                        exit(1);
                    }
                    printf("Ok received from client!\n");
                    fclose(recvFile);
                }


            }
            else
            {
                cout << "Error: listen() < 0" << endl;
            }
        }
        else
        {
            cout << "Error: Unable to bind" << endl;
        }
    }
    else
    {
        cout << "Error: Unable to create socket" << endl;
    }
}

int main (int argc, char* argv[]){
    if (argc == 1)
    {
        server();
    }

    else if (argc == 2 && strcmp(argv[1],"-h") == 0)
    {
        printf("%s\n", "Usage:");
        printf("%s\n", "Server: ./chat");
        printf("%s\n", "Client: ./chat -p port -s server_ip");
    }
    else if (argc == 5 && strcmp(argv[1],"-p") == 0 && strcmp(argv[3],"-s") == 0 )
    {
        client(argv[4],argv[2]);
    }
    else if (argc == 5 && strcmp(argv[3],"-p") == 0 && strcmp(argv[1],"-s") == 0 )
    {
        client(argv[2],argv[4]);
    }
    else
    {
        printf("%s\n","Unknown option 102. Try ./chat -h for help.");
    }

    return 0;
}