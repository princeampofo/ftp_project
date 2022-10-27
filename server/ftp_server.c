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
#define CLIENT_MSG_SIZE 1024
#define LOGGED_IN_SIZE 20

typedef struct login_info{
    char u_name[15];
    char p_word[15];
} LOGIN_INFO;

void execute_client_command(int sock, char* client_command);
void load_server_logins();

LOGIN_INFO logins_array[LOGGED_IN_SIZE];
int loggedIn_users[1024];
char loggingIn_users[50][50];

int main(){

    
    // load logins from file to logins array
    load_server_logins();
    // memset(loggingIn_users, NULL, sizeof(loggingIn_users));
    memset(loggedIn_users, 0, sizeof(loggedIn_users));
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
    printf("Server is running on PORT 21...\n");
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
                    char msg_to_client[] = "220 Service ready for new user.\n";
                    send(client_socket,msg_to_client,sizeof(msg_to_client),0);
                }
                
                // if a client socket is set ,get command and execute it
                else{
                    char *cmd_from_client = (char*)malloc(CLIENT_MSG_SIZE);
                    bzero(cmd_from_client, CLIENT_MSG_SIZE);
                    printf("Before: %s with %d\n", cmd_from_client, CLIENT_MSG_SIZE);
                    // receive message
                    recv(fd,cmd_from_client,CLIENT_MSG_SIZE,0);
                    printf("After: %s with %lu\n",cmd_from_client, sizeof(cmd_from_client));
                    // if message is Quit or zero in length , close connection and remove from current socket set
                    if (strcmp(cmd_from_client,"QUIT")==0 || strlen(cmd_from_client)==0){
                        char client_msg[]="221 Service closing control connection\n";
                        send(fd,client_msg,sizeof client_msg,0);
                        close(fd);
                        FD_CLR(fd,&current_sockets);
                        break;  
                    }
                    else{

                        execute_client_command(fd,cmd_from_client);
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
void execute_client_command(int sock,  char* client_command){
    char* cmd_client = strtok(client_command," ");
    char* arg_client = strtok(NULL,"");
    // check if user is logged in
    if (strcmp(cmd_client,"USER")==0){
        if (loggedIn_users[sock]==1 || strcmp(loggingIn_users[sock],"")!=0 || arg_client==NULL){
            char msg_to_client[] = "503 Bad sequence of commands.";
            send(sock,msg_to_client,sizeof(msg_to_client),0);
        }
        else{
            for (int i = 0; i < LOGGED_IN_SIZE; i++){
                if (strcmp(logins_array[i].u_name,arg_client)==0){
                    char msg_to_client[] = "331 User name okay, need password.";
                    send(sock,msg_to_client,sizeof(msg_to_client),0);
                    // loggingIn_users[sock] = arg_client;
                    strcpy(loggingIn_users[sock],arg_client);
                    return;
                }
            }
            char msg_to_client[] = "530 Not logged in.";
            send(sock,msg_to_client,sizeof(msg_to_client),0);
        }
        return;
    }
    else if(strcmp(cmd_client,"PASS")==0){
        // check user password
        if (strcmp(loggingIn_users[sock],"")==0 || arg_client==NULL){
            char msg_to_client[] = "503 Bad sequence of commands.";
            send(sock,msg_to_client,sizeof(msg_to_client),0);
        }
        else{
            printf("%s\n", loggingIn_users[sock]);
            printf("%s\n", arg_client);
            printf("--------------------\n");
            for (int i = 0; i < LOGGED_IN_SIZE; i++)
            {
                // printf("%s\n", logins_array[i].u_name);
                // printf("%s\n", logins_array[i].p_word);
                if (strcmp(loggingIn_users[sock],logins_array[i].u_name)==0 && strcmp(logins_array[i].p_word,arg_client)==0){
                    char msg_to_client[] = "230 User logged in, proceed.";
                    send(sock,msg_to_client,sizeof(msg_to_client),0);
                    loggedIn_users[sock]=1;
                    strcpy(loggingIn_users[sock], "");
                    // loggingIn_users[sock]=NULL;
                    return;
                }
            }
            char msg_to_client[] = "530 Not logged in.";
            send(sock,msg_to_client,sizeof(msg_to_client),0);
        }
        return;
    }
    if (loggedIn_users[sock]==0){
        char msg_to_client[] = "530 Not logged in.";
        send(sock,msg_to_client,sizeof(msg_to_client),0);
        return;
    }
    if (strcmp(cmd_client,"LIST")==0){
        if(arg_client!=NULL){
            char client_msg[]="503 Bad sequence of commands.";
            send(sock,client_msg,sizeof client_msg,0);
        }
        else{
            char client_msg[]="150 File status okay; about to open data connection.";
            send(sock,client_msg,sizeof client_msg,0);
            // get list of files in current directory
            char list_cmd[100];
            strcpy(list_cmd,"ls ");
            strcat(list_cmd,arg_client);
            strcat(list_cmd," > list.txt");
            system(list_cmd);
            // send list of files to client
            FILE *fp = fopen("list.txt","r");
            char file_list[1000];
            bzero(file_list, sizeof file_list);
            fread(file_list,1,1000,fp);
            send(sock,file_list,sizeof file_list,0);
            fclose(fp);
            char client_msg2[]="226 Transfer Completed\n";
            send(sock,client_msg2,sizeof client_msg2,0);
        } 

    }
    else if (strcmp(cmd_client,"RETR")==0){
        if(arg_client==NULL){
            char client_msg[]="503 Bad sequence of commands.";
            send(sock,client_msg,sizeof client_msg,0);
        }
        else{
            char client_msg[]="150 File status okay; about to open data connection.";
            send(sock,client_msg,sizeof client_msg,0); 
            // send file to client
            FILE *fp = fopen(arg_client,"rb");
            if (fp==NULL){
                perror("Error opening file\n");
                // send error message to client
                char client_msg[]="550 Requested action not taken. File unavailable.";
                send(sock,client_msg,sizeof client_msg,0);
            }
            else{
                char file_content[1000];
                bzero(file_content, sizeof file_content);
                fread(file_content,1,1000,fp);
                send(sock,file_content,sizeof file_content,0);
                fclose(fp);
                char client_msg2[]="226 Transfer Completed\n";
                send(sock,client_msg2,sizeof client_msg2,0);
            }
        } 
    }
    else if (strcmp(cmd_client,"STOR")==0){
        if(arg_client==NULL){
            char client_msg[]="503 Bad sequence of commands.";
            send(sock,client_msg,sizeof client_msg,0);
        }
        else{
            char client_msg[]="150 File status okay; about to open data connection.";
            send(sock,client_msg,sizeof client_msg,0);
            // get file from client
            char file_stor[1000];
            bzero(file_stor, sizeof file_stor);
            recv(sock,file_stor,sizeof file_stor,0);
            // store file in current directory
            FILE *fp = fopen(arg_client,"wb");
            fwrite(file_stor,1,1000,fp);
            fclose(fp);
            char client_msg2[]="226 Transfer Completed";
            send(sock,client_msg2,sizeof client_msg2,0);
        } 
    }
}

void load_server_logins(){
    FILE *fp= fopen("users.txt","r");

    if (fp!=NULL){
        int count = 0;
        char file_data[50];
        memset(file_data,'0',sizeof file_data);

        while(fgets(file_data,sizeof file_data , fp)!=NULL){
            char *token = strtok(file_data,",");
            strcpy(logins_array[count].u_name, token);
            strcpy(logins_array[count].p_word, strtok(NULL,"\n"));
            count ++;
        }
    }
    else{
        perror("Error opening file for reading!\n");
        exit(0);
    }

}