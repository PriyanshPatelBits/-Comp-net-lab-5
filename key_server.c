#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define MAXPENDING 5
#define BUFFERSIZE 64
#define FILESIZE 1024

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
    serverAddress.sin_port = htons(12345);
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
            //printf("%d", 1);
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddress.sin_addr),
        
        ntohs(clientAddress.sin_port));
        if((childpid = fork()) == 0){
            close(serverSocket);
            while(1){
                recv(newSocket, msg, BUFFERSIZE, 0);
                if(strcmp(msg, "Bye") == 0){
                    printf("Client: %s\n", msg);
                    char ret[BUFFERSIZE] = "Goodbye";
                    send(newSocket, ret, strlen(ret), 0);
                    bzero(msg, sizeof(msg));
                    printf("Disconnected from %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                    break;
                }
                else {
                    printf("Client: %s\n", msg);

                    char command[4];
                    for(int i = 0; i<3; i++){
                        command[i] = msg[i];
                    }
                    command[3] = '\0';

                    if(strcmp(command, "get") == 0){
                        char key[BUFFERSIZE];
                        int ksize = 0;
                        for(int i = 4; i<BUFFERSIZE; i++){
                            if(msg[i]==' '){
                                break;
                            }
                            key[i-4] = msg[i];
                            ksize++;
                        }
                        key[ksize] = '\0';

                        FILE * fptr = fopen("database.txt", "r");
                        if(fptr==NULL){
                            printf("Error in opening file, please try again later!\n");
                            continue;
                        }

                        char curr_key[BUFFERSIZE];
                        char value[BUFFERSIZE];

                        bool flag = 0;

                        while(fscanf(fptr, "%s", curr_key)>0){
                            
                            fscanf(fptr, "%s", value);

                            if(strcmp(curr_key, key)==0){
                                flag = 1;
                                break;
                            }
                        }

                        if(flag){
                            send(newSocket, value, strlen(value), 0);
                            bzero(msg, sizeof(msg));
                        }

                        else{
                            char ret[BUFFERSIZE] = "Key not found";
                            send(newSocket, ret, strlen(ret), 0);
                            bzero(msg, sizeof(msg));
                        }

                        fclose(fptr);
                    }

                    else if(strcmp(command, "put") == 0){
                        char key[BUFFERSIZE];
                        int ksize = 0;
                        for(int i = 4; i<strlen(msg); i++){
                            if(msg[i]==' '){
                                break;
                            }
                            key[i-4] = msg[i];
                            ksize++;
                        }
                        key[ksize] = '\0';

                        char val[BUFFERSIZE];
                        int vsize = 0;
                        for(int i = ksize+5; i<strlen(msg); i++){
                            if(msg[i]== ' '){
                                break;
                            }
                            val[i-ksize-5] = msg[i];
                            vsize++;
                        }
                        val[vsize] = '\0';

                        FILE * fptr = fopen("database.txt", "r+");
                        if(fptr==NULL){
                            printf("Error in opening file, please try again later!\n");
                            continue;
                        }

                        char curr_key[BUFFERSIZE];
                        char value[BUFFERSIZE];

                        bool flag = 0;

                        while(fscanf(fptr, "%s", curr_key)>0){
                            
                            fscanf(fptr, "%s", value);

                            if(strcmp(curr_key, key)==0){
                                flag = 1;
                                break;
                            }
                        }

                        if(flag){
                            char ret[BUFFERSIZE] = "Key already exists";
                            send(newSocket, ret, strlen(ret), 0);
                            bzero(msg, sizeof(msg));
                        }

                        else{
                            fprintf(fptr, "%s %s\n", key, val);
                            char ret[BUFFERSIZE] = "OK";
                            send(newSocket, ret, strlen(ret), 0);
                            bzero(msg, sizeof(msg));
                        }

                        fclose(fptr);

                    }

                    else if(strcmp(command, "del") == 0){
                        char key[BUFFERSIZE];
                        int ksize = 0;
                        for(int i = 4; i<BUFFERSIZE; i++){
                            if(msg[i]==' '){
                                break;
                            }
                            key[i-4] = msg[i];
                            ksize++;
                        }
                        key[ksize] = '\0';

                        FILE * fptr = fopen("database.txt", "r");
                        if(fptr==NULL){
                            printf("Error in opening file, please try again later!\n");
                            continue;
                        }

                        char curr_key[BUFFERSIZE];
                        char value[BUFFERSIZE];

                        char updated[FILESIZE];
                        memset (updated, '\0', sizeof(updated));

                        bool flag = 0;

                        while(fscanf(fptr, "%s", curr_key)>0){
                            
                            fscanf(fptr, "%s", value);

                            if(strcmp(curr_key, key)==0){
                                flag = 1;
                                continue;
                            }

                            strcat(updated, curr_key);
                            strcat(updated, " ");
                            strcat(updated, value);
                            strcat(updated, "\n");

                            //printf("%s\n", updated);
                        }

                        fclose(fptr);
                        if(flag){
                            fptr = fopen("database.txt", "w");
                            if(fptr==NULL){
                                printf("Error in opening file, please try again later!\n");
                                continue;
                            }
                            //printf("%s\n", updated);
                            fprintf(fptr, "%s", updated);
                            fclose(fptr);

                            char ret[BUFFERSIZE] = "OK";
                            send(newSocket, ret, strlen(ret), 0);
                            bzero(msg, sizeof(msg));
                        }

                        else{
                            char ret[BUFFERSIZE] = "Key not found";
                            send(newSocket, ret, strlen(ret), 0);
                            bzero(msg, sizeof(msg));
                        }
                    }
                    else{
                        char ret[BUFFERSIZE] = "Enter valid command";
                        send(newSocket, ret, strlen(ret), 0);
                        bzero(msg, sizeof(msg));
                    }
                }
            }
        }
    }
}