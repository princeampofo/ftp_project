#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include<arpa/inet.h>
#include <string.h>

#define CONTROL_PORT 21
#define DATA_PORT 20
#define IP_ADDRESS "127.0.0.1"
#define CLIENT_MSG_SIZE 200
#define LOGGED_IN_SIZE 20
#define FILE_BUFFER_SIZE 1024

int client_port;
char client_ip[20];

typedef struct login_info{
    char u_name[15];
    char p_word[15];
} LOGIN_INFO;

void load_server_logins();
void bindAndlisten(int server_socket, struct sockaddr_in server_address);
void execute_client_command(int sock, char* client_command);
void checkUser(int sock, char* client_command);
void checkPassword(int sock, char* client_command);
void sendWorkingDirectory(int sock, char* client_command);
void changeWorkingDirectory(int sock, char* client_command);
int dataConnection(char* IP, long port);
void sendFile(int sock, char* client_command, int data_sock);
void receiveFile(int sock, char* client_command, int data_sock);
void listDirectory(int sock, char* client_command, int data_sock);


LOGIN_INFO logins_array[LOGGED_IN_SIZE];
int loggedIn_users[1024];
char loggingIn_users[50][50];

int main(){

    // change the current working directory to the server directory
    chdir("server");

    // load logins from file to logins array
    load_server_logins();
    // create a socket(end point for communication)
    memset(loggedIn_users, 0, sizeof(loggedIn_users));

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
    server_addr.sin_port = htons(CONTROL_PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    bindAndlisten(server_socket, server_addr);

    // create fd_sets and zero out each fd_set
    fd_set current_sockets, ready_sockets;
    FD_ZERO(&current_sockets);
    FD_ZERO(&ready_sockets);

    // add server socket to current_sockets

    FD_SET(server_socket,&current_sockets);
    for (;;)
    {
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
                    // receive message
                    recv(fd,cmd_from_client,CLIENT_MSG_SIZE,0);

                    if(strncmp(cmd_from_client, "QUIT", 4) == 0 || strlen(cmd_from_client) == 0){
                        char msg_to_client[] = "221 Service closing control connection.\n";
                        send(fd,msg_to_client,sizeof(msg_to_client),0);
                        loggedIn_users[fd]=0;
                        FD_CLR(fd,&current_sockets);
                        break;
                    }

                    // if command is RETR or STOR or LIST , get the client port
                    if(strncmp(cmd_from_client, "RETR", 4) == 0 || strncmp(cmd_from_client, "STOR", 4) == 0 || strncmp(cmd_from_client, "LIST", 4) == 0){
                        // fork a child process
                        int pid = fork();
                        if (pid == 0){
                            // close the server socket
                            close(server_socket);
                            // execute command
                            execute_client_command(fd,cmd_from_client);
                            exit(0);
                        }
                        else{
                            // do none in parent process
                        }
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

void load_server_logins(){
    FILE *fp= fopen("users.txt","r");
    if(fp == NULL){
        printf("Error opening file\n");
        exit(1);
    }
    int count = 0;
    char file_data[50];
    memset(file_data,'0',sizeof file_data);

    while(fgets(file_data,sizeof file_data , fp)!=NULL){
        char *token = strtok(file_data,",");
        strcpy(logins_array[count].u_name, token);
        strcpy(logins_array[count].p_word, strtok(NULL, "\n"));
        count ++;
    }
    fclose(fp);
}

void bindAndlisten(int server_socket, struct sockaddr_in server_address){
    if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        printf("Error binding socket\n");
        exit(1);
    }
    //Listen for connections
    if(listen(server_socket, 5) < 0){
        printf("Error listening\n");
        exit(1);
    }
    printf("Server is running on PORT %d...\n", CONTROL_PORT);
}

void checkUser(int sock, char* arg){
    if (loggedIn_users[sock]==1 || strcmp(loggingIn_users[sock],"")!=0 || arg==NULL){
            char msg_to_client[] = "503 Bad sequence of commands.\n";
            send(sock,msg_to_client,sizeof(msg_to_client),0);
    }
    else{
        for (int i = 0; i < LOGGED_IN_SIZE; i++){
            if (strcmp(logins_array[i].u_name,arg)==0){
            char msg_to_client[] = "331 User name okay, need password.\n";
            send(sock,msg_to_client,sizeof(msg_to_client),0);
            strcpy(loggingIn_users[sock],arg);
            return;
            }
        }
        char msg_to_client[] = "530 Not logged in.\n";
        send(sock,msg_to_client,sizeof(msg_to_client),0);
    }
}

void checkPassword(int sock, char* arg){
    if (loggedIn_users[sock] == 1 || strcmp(loggingIn_users[sock], "") == 0 || arg == NULL)
    {
        char msg_to_client[] = "503 Bad sequence of commands.\n";
        send(sock, msg_to_client, sizeof(msg_to_client), 0);
    }
    else{
        for (int i = 0; i < LOGGED_IN_SIZE; i++)
        {
            if (strcmp(logins_array[i].u_name,loggingIn_users[sock])==0 && strcmp(logins_array[i].p_word,arg)==0){
            char msg_to_client[] = "230 User logged in, proceed.\n";
            loggedIn_users[sock]=1;
            strcpy(loggingIn_users[sock],"");
            if(send(sock,msg_to_client,sizeof(msg_to_client),0) < 0){
                printf("Error sending message to client\n");
            }
            return;
            }
        }
        char msg_to_client[] = "530 Not logged in.\n";
        send(sock,msg_to_client,sizeof(msg_to_client),0);
    }
    return;
}

void sendWorkingDirectory(int sock, char* arg){
    if(arg!=NULL){
        char client_msg1[]="501 Syntax error in parameters or arguments.\n";
        send(sock,client_msg1,sizeof client_msg1,0);
    }
    else{
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        char msg_to_client[1024];
        sprintf(msg_to_client, "257 \"%s\"\n", cwd);
        send(sock, msg_to_client, sizeof(msg_to_client), 0);
    }
}

void changeWorkingDirectory(int sock, char* arg){
    if(arg==NULL){
        char client_msg1[]="501 Syntax error in parameters or arguments.\n";
        send(sock,client_msg1,sizeof client_msg1,0);
    }
    else{
        char newarg[1024];
        sprintf(newarg, "./%s", arg);
        if(chdir(newarg)==0){
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            char msg_to_client[1024];
            sprintf(msg_to_client, "200 directory changed to \"%s\"\n", cwd);
            send(sock, msg_to_client, sizeof(msg_to_client), 0);
        }
        else{
            char client_msg1[]="550 No such file or directory.\n";
            send(sock,client_msg1,sizeof client_msg1,0);
        }
    }
}

int dataConnection(char* ip, long port){
    int data_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(data_sock < 0){
        printf("Error creating socket\n");
        exit(1);
    }

    // allow reuse of port
    int optval = 1;
    setsockopt(data_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    // Address structure of client for data transfer
    struct sockaddr_in client_sock_addr, server_sock_addr;
    memset(&client_sock_addr,0, sizeof(client_sock_addr));
    memset(&server_sock_addr,0, sizeof(server_sock_addr));

    client_sock_addr.sin_family = AF_INET;
    client_sock_addr.sin_port = htons(port);
    client_sock_addr.sin_addr.s_addr = inet_addr(ip);

    server_sock_addr.sin_family = AF_INET;
    server_sock_addr.sin_port = htons(DATA_PORT);
    server_sock_addr.sin_addr.s_addr = inet_addr(ip);

    // bind
    if (bind(data_sock, (struct sockaddr*) &server_sock_addr,sizeof(server_sock_addr))<0){
        perror("Error binding to socket!\n");
        exit(0);
    }
    // connect to client
    if (connect(data_sock, (struct sockaddr*) &client_sock_addr, sizeof(client_sock_addr))<0){
        perror("Error connecting to client!\n");
    }
    return data_sock;
}

void sendFile(int sock, char* arg, int data_sock){
    if(arg==NULL){
        char client_msg1[]="501 Syntax error in parameters or arguments.\n";
        send(sock,client_msg1,sizeof client_msg1,0);
        close(data_sock);
    }
    else{
        FILE *fp = fopen(arg, "rb");
        if(fp == NULL){
            char client_msg1[]="550 No such file or directory.\n";
            send(sock,client_msg1,sizeof client_msg1,0);
            close(data_sock);
        }
        else{
            char client_msg1[]="150 File status okay; about to open data connection.\n";
            send(sock,client_msg1,sizeof client_msg1,0);
            char buffer[FILE_BUFFER_SIZE];
            bzero(buffer, sizeof(buffer));
            int bytes_read;
            while((bytes_read = fread(buffer, 1, FILE_BUFFER_SIZE, fp)) > 0){
            send(data_sock, buffer, bytes_read, 0);
            bzero(buffer, sizeof(buffer));
            }
            close(data_sock);
            fclose(fp);
            char client_msg2[] = "226 Transfer completed.\n";
            send(sock,client_msg2,sizeof client_msg2,0);
        }
    }
}

void receiveFile(int sock, char* arg, int data_sock){
    if(arg==NULL){
        char client_msg1[]="501 Syntax error in parameters or arguments.\n";
        send(sock,client_msg1,sizeof client_msg1,0);
        close(data_sock);
    }
    else{
        int randNumber = rand() % 1000;
        char *tempFileName = malloc(100);
        sprintf(tempFileName, "%d%s", randNumber, arg);
        FILE *tempFile = fopen(tempFileName, "wb");
        if(tempFile == NULL){
            perror("Could not create temp file.\n");
        }
        char buffer[FILE_BUFFER_SIZE];
        bzero(buffer, sizeof(buffer));
        int bytes_read;
        while((bytes_read = recv(data_sock, buffer, FILE_BUFFER_SIZE, 0)) > 0){
            fwrite(buffer, 1, bytes_read, tempFile);
            bzero(buffer, sizeof(buffer));
        }
        fclose(tempFile);
        rename(tempFileName, arg);
        close(data_sock);

        char client_msg2[] = "226 Transfer completed.\n";
        send(sock,client_msg2,sizeof client_msg2,0);
    }
}

void listDirectory(int sock, char* arg, int data_sock){

    char buffer[FILE_BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));

    char command[1024];
    if(arg==NULL){
        sprintf(command, "ls");
    }
    else{
        if(access(arg, F_OK) < 0){
            char client_msg1[]="550 No such file or directory.\n";
            send(sock,client_msg1,sizeof client_msg1,0);
            close(data_sock);
            return;
        }
        sprintf(command, "ls %s", arg);
    }
    FILE *fp = popen(command, "r");

    char client_msg1[] = "150 File status okay; about to open data connection.\n";
    send(sock,client_msg1,sizeof client_msg1,0);

    while(fgets(buffer, FILE_BUFFER_SIZE, fp) != NULL){
        send(data_sock, buffer, strlen(buffer), 0);
        bzero(buffer, sizeof(buffer));
    }
    pclose(fp);
    close(data_sock);
    char client_msg2[] = "226 Transfer completed.\n";
    send(sock,client_msg2,sizeof client_msg2,0);
}

void execute_client_command(int sock, char* client_command){
    char* command = strtok(client_command, " ");
    char* arg = strtok(NULL, " ");
    if(strcmp(command, "USER") == 0){
        //Check if user exists
        checkUser(sock, arg);
        return;
    }
    else if(strcmp(command, "PASS")== 0){
        //Check if password is correct
        checkPassword(sock, arg);
        return;
    }

    if (loggedIn_users[sock]==0){
        char msg_to_client[] = "530 Not logged in.\n";
        send(sock,msg_to_client,sizeof(msg_to_client),0);
        return;
    }
    if(strcmp(command, "PWD") == 0){
        //If command is PWD
        sendWorkingDirectory(sock, arg);
        return;
    }
    else if(strcmp(command, "CWD") == 0){
        //Change working directory
        changeWorkingDirectory(sock, arg);
        return;
    }
    else if(strcmp(command, "PORT") == 0){
        //Commands that use port
        char IP[32];
        char p1[8];
        char p2[8];
        bzero(IP,sizeof IP);
        bzero(p1,sizeof p1);
        bzero(p2,sizeof p2);
        char* token = strtok(arg, ",");
        for (int i=0; i<4;i++){
            strcat(IP,token);
            if (i<3){
                strcat(IP,".");
                token = strtok(NULL,",");
            }
        }
        strcpy(p1,strtok(NULL,","));
        strcpy(p2,strtok(NULL, ""));

        bzero(client_ip, sizeof(client_ip));
        strcpy(client_ip, IP);
        long p1val = strtol(p1,NULL,10);
        long p2val = strtol(p2,NULL,10);
        client_port = p1val*256+p2val;

        //Send port command successful to client
        char msg_to_client[]= "200 PORT command successful.\n";
        send(sock, msg_to_client, sizeof(msg_to_client), 0);

        return;
    }
    // check if command is RETR or STOR or LIST
    else if(strcmp(command, "RETR") == 0 || strcmp(command, "STOR") == 0 || strcmp(command, "LIST") == 0){
        //Open data connection to client
        int data_sock = dataConnection(client_ip, client_port);

        if(strcmp(command, "RETR") == 0){
            //If command is RETR
            sendFile(sock, arg, data_sock);
            return;
        }
        else if(strcmp(command, "STOR") == 0){
            //If command is STOR
            receiveFile(sock, arg, data_sock);
            return;
        }
        else if(strcmp(command, "LIST") == 0){
            //If command is LIST
            listDirectory(sock, arg, data_sock);
            return;
        }

    }
    else{
        char msg_to_client[] = "502 Command not implemented.\n";
        send(sock,msg_to_client,sizeof(msg_to_client),0);
        return;
    }
}
