#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 21
#define IP_ADDRESS "127.0.0.1"
#define CLIENT_INPUT_SIZE 100
#define SERVER_RESPONSE_SIZE 300

void execute_ftp_command(int sock, char* server_response);

int main(){

    // // create a socket(end pointt for communication)
    int client_socket = socket(AF_INET,SOCK_STREAM,0);

    // // if the socket return value is -1 , there was an error creating it
    // if (client_socket<0){
    //     perror("Couldn't create socket\n");
    //     exit(0);
    // }

    // // Address structure of server
    // struct sockaddr_in server_addr;
    // bzero(&server_addr,sizeof(server_addr));
    // server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(PORT);
    // server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    // // connect 
    // if(connect(client_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
    //     perror("Error connecting socket to address specified!\n");
    //     exit(0);
    // }
    char serv_connection_response[SERVER_RESPONSE_SIZE];
	// if (recv(client_socket, serv_connection_response, sizeof(serv_connection_response), 0) < 0)
	// {
	// 	perror("ERROR in recvfrom");
	// }
    // printf("%s", serv_connection_response);

    //execute ftp command
	execute_ftp_command(client_socket, serv_connection_response);
    
    close(client_socket);


    return EXIT_SUCCESS;
}

void execute_locally(char* input_command){
    char* client_command = strtok(input_command, " ");
    char* client_argument = strtok(NULL, " ");

    if (strcmp(client_command, "!LIST") == 0){
        if (client_argument != NULL){
            printf("Invalid command\n");
        }
        else system("ls");
    }
    else if (strcmp(client_command, "!CWD") == 0){
        if (client_argument == NULL){
            printf("No folder name provided!\n");
        }
        else{
            char command[256];
            strcpy(command, "cd ");
            strcat(command, client_argument);
            system(command);
        }
    }
    else if (strcmp(client_command, "!PWD") == 0){
        if (client_argument != NULL){
            printf("Invalid command\n");
        }
        else system("pwd");
    }
    else{
        printf("Invalid command\n");
    }
}

void execute_ftp_command(int sock, char* server_response){
    char client_input[CLIENT_INPUT_SIZE];
    int char_count;
    do{
        bzero(server_response,SERVER_RESPONSE_SIZE);
        do{
            bzero(client_input,sizeof(client_input));
            printf("ftp> ");
            char_count = 0;
            char c;
            while ((c = getchar()) != '\n')
            {
                client_input[char_count++] = c;
            }
            if (strlen(client_input)==0){
                printf("No command entered!\n");
            }
            else if(client_input[0]=='!'){
                // execute command locally
                execute_locally(client_input);
            }
            else{
                // send command to server
                if (send(sock, client_input, sizeof(client_input), 0) < 0)
                {
                    perror("ERROR in sendto");
                }
                // receive response from server
                if (recv(sock, server_response,SERVER_RESPONSE_SIZE, 0) < 0)
                {
                    perror("ERROR in recvfrom");
                }
                printf("%s", server_response);
            }
        }while(strlen(client_input)!=0);
    }while(strcmp(client_input,"QUIT") !=0);

}
