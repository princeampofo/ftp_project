#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include<arpa/inet.h>
#include <string.h>
#include "../parse.h"

#define PORT 21
#define IP_ADDRESS "127.0.0.1"

typedef struct login_info{
    char u_name[15];
    char p_word[15];
} LOGIN_INFO;

void execute_client_command();
void load_server_logins();

LOGIN_INFO logins_array[20];

int main(){

    
    // load logins from file to logins array
    load_server_logins();
    // create a socket(end point for communication)
    int server_socket = socket(AF_INET,SOCK_STREAM,0);

    // if the socket return value is -1 , there was an error creating it
    if (server_socket<0){
        perror("Couldn't create socket\n");
        exit(0);
    }

    //allows reuse of address
	int value = 1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(int)); 

    // Address structure of server
    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(server_addr);
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    // bind socket tp address
    if(bind(server_socket,(struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        perror("Binding failed!\n");
        exit(0);
    }

    // listening for connections
    if(listen(server_socket,5)<0){
        perror("Listening failed!\n");
        close(server_socket);
        exit(0);
    }

    while(1){

    }

    // close server socket
    close(server_socket);

    return EXIT_SUCCESS;
}

// execute client command
void execute_client_command(){

}

void load_server_logins(){
    FILE *fp= fopen("../users.txt","r");

    if (fp!=NULL){
        int count = 0;
        char file_data[50];
        memset(file_data,'0',sizeof file_data);

        while(fgets(file_data,sizeof file_data , fp)!=NULL){
            char *token = strtok(file_data,",");
            strcpy(logins_array[count].u_name, token);
            strcpy(logins_array[count].p_word,token = strtok(NULL,"\n"));
            count ++;
        }
    }
    else{
        perror("Error opening file for reading!\n");
        exit(0);
    }

}