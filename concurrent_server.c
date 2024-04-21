#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXPENDING 5
#define BUFFERSIZE 32
int main ()
{
    /*CREATE A TCP SOCKET*/
    int serverSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) { printf ("Error while server socket creation"); exit (0); }
    printf ("Server Socket Created\n");

    /*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
    struct sockaddr_in serverAddress, clientAddress;
    memset (&serverAddress, 0, sizeof(serverAddress));
    
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12347);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    printf ("Server address assigned\n");
    
    int temp = bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if (temp < 0)
    { 
        printf ("Error while binding\n");
        exit (0);
    }
    printf ("Binding successful\n");
    
    int temp1 = listen(serverSocket, MAXPENDING);
    if (temp1 < 0)
    {   printf ("Error in listen");
        exit (0);
    }
    printf ("Now Listening\n");

    int newSocket;
    int childpid;
    char msg[BUFFERSIZE];

    while(1){
        int clientLength = sizeof(clientAddress);
        newSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);
        if(newSocket < 0){
            printf("%d", 1);
            exit(1);
        }
        printf("Connection accepted from %s:%d\n",inet_ntoa(clientAddress.sin_addr),
        
        ntohs(clientAddress.sin_port));
        if((childpid = fork()) == 0){
            close(serverSocket);
            while(1){
                recv(newSocket, msg, BUFFERSIZE, 0);
                if(strcmp(msg, ":exit") == 0){
                    printf("Disconnected from %s:%d\n",inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                    break;
                }
                else {
                    printf("Client: %s\n", msg);
                    send(newSocket, msg, strlen(msg), 0);
                    bzero(msg, sizeof(msg));
                }
            }
        }
    }
}