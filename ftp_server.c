#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include<arpa/inet.h>
#include <string.h>

#define PORT 21
#define IP_ADDRESS "127.0.0.1"
#define CLIENT_MSG_SIZE 200

typedef struct login_info{
    char u_name[15];
    char p_word[15];
} LOGIN_INFO;

void execute_client_command(int sock);
void load_server_logins();

LOGIN_INFO logins_array[20];
int logedIn_users[1024];
int loggingIn_users[1024];

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

    // create fd_sets and zero out each fd_set
    fd_set current_sockets, ready_sockets;
    FD_ZERO(&current_sockets);
    FD_ZERO(&ready_sockets);

    // add server socket to current_sockets
    FD_SET(server_socket,&current_sockets);
    for(;;){
        ready_sockets = current_sockets;

        // monitor ready sockets set
        if (select(FD_SETSIZE,&ready_sockets,NULL,NULL,NULL) < 0){
            perror("Error monitoring socket sets\n");
            exit(0);
        }

        // loop through ready soskets and handle connections
        int fd = 0;
        for(;fd<FD_SETSIZE;fd++){
            if (FD_ISSET(fd, &ready_sockets)){
                // if the server socket is set , accept connection and add to current socket
                if (fd==server_socket){
                    int client_socket = accept(server_socket,(struct sockaddr *) &client_addr,(socklen_t *)&addrlen);
                     if(client_socket<0){
                        exit(0);
                    }

                    FD_SET(client_socket,&current_sockets);
                    char msg_to_client[] = "220 Service ready for new user.";
                    send(client_socket,msg_to_client,sizeof(msg_to_client),0);
                }
                
                // if a client socket is set ,get command and execute it
                else{
                    char cmd_from_client[CLIENT_MSG_SIZE];
                    bzero(cmd_from_client, sizeof cmd_from_client);
                    // receive message
                    recv(fd,cmd_from_client,sizeof cmd_from_client,0);

                    // if message is Quit or zero in length , close connection and remove from current socket set
                    if (strcmp(cmd_from_client,"QUIT")==0 || strlen(cmd_from_client)==0){
                        char client_msg[]="221 Service closing control connection";
                        send(fd,client_msg,sizeof client_msg,0);

                        close(fd);
                        FD_CLR(fd,&current_sockets);
                        break;  
                    }
                    else{

                        execute_client_command(fd);
                    }

                }
            }
        }
    }

    // close server socket
    close(server_socket);

    return EXIT_SUCCESS;
}

// execute client command
void execute_client_command(int sock){

}

void load_server_logins(){
    FILE *fp= fopen("./users.txt","r");

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