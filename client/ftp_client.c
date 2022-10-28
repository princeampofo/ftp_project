#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// #define PORT 8080
#define IP_ADDRESS "127.0.0.1"
#define CLIENT_INPUT_SIZE 100
#define SERVER_RESPONSE_SIZE 1500
#define FILE_BUFFER_SIZE 1024

void execute_locally(char* command);
void retrieveFile(char* filename, int server_sock, int data_sock);
void sendFile(char* filename, int server_sock, int data_sock, FILE* fp);
void listFile(char* args, int server_sock, int data_sock);

int main(){
   srand(time(NULL));
   // change directory to client
   chdir("client");

   // Create socket
   int sock = socket(AF_INET, SOCK_STREAM, 0);
   if(sock < 0){
      printf("Error creating socket\n");
      exit(1);
   }

   //Randomize port number
   int PORT = rand() % 1000 + 8000;

   // Client address
   struct sockaddr_in client_address;
   client_address.sin_family = AF_INET;
   client_address.sin_port = htons(PORT);
   client_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);
   
   //Server address
   struct sockaddr_in server_address;
   server_address.sin_family = AF_INET;
   server_address.sin_port = htons(21);
   server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);

   // Connect to server
   if(connect(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
      printf("Error connecting to server\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   printf("%s", server_response);
   
   //Get commands to execute
   char command[CLIENT_INPUT_SIZE];
   int client_ports = PORT;
   while (1)
   {
      bzero(server_response, SERVER_RESPONSE_SIZE);
      printf("ftp> ");
      fgets(command, CLIENT_INPUT_SIZE, stdin);
      command[strlen(command) - 1] = '\0';
      if(command[0] == '!'){
         execute_locally(command);
      }
      // Send command to server
      else {
         //If command requires PORT to be sent
         if (strncmp(command, "RETR", 4) == 0 || strncmp(command, "STOR", 4) == 0 || strncmp(command, "LIST", 4) == 0){
            client_ports += 1;
            // Create the ports to send
            char portCommand[100] = "PORT ";
            char IP[] = IP_ADDRESS;
            char* ipToken = strtok(IP, ".");
            while(ipToken != NULL){
               strcat(portCommand, ipToken);
               strcat(portCommand, ",");
               ipToken = strtok(NULL, ".");
            }
            // Ports for data transfer
            unsigned int p1 = client_ports / 256; // Higher byte of port
            unsigned int p2 = client_ports % 256; // Lower byte of port
            char ports[8];
            sprintf(ports, "%d", p1);
            strcat(portCommand, ports);
            strcat(portCommand, ",");
            sprintf(ports, "%d", p2);
            strcat(portCommand, ports);

            // create a socket for data transfer
            int data_sock = socket(AF_INET,SOCK_STREAM,0);
            // if the socket return value is -1 , there was an error creating it
            if (data_sock < 0){
                  perror("Couldn't create socket\n");
                  exit(0);
            }

                //allows reuse of address
                int value = 1;
                setsockopt(data_sock,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(int)); 

                // Address structure of server
                struct sockaddr_in client_sock_addr, server_addr;
                int addrlen = sizeof(client_sock_addr);
                bzero(&client_sock_addr,sizeof(client_sock_addr));
                bzero(&server_addr,sizeof(server_addr));


                client_sock_addr.sin_family = AF_INET;
                client_sock_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
                client_sock_addr.sin_port = htons(client_ports);

            // bind socket tp address
            if(bind(data_sock, (struct sockaddr*) &client_sock_addr, sizeof(client_sock_addr)) < 0){
                  perror("Binding failed!\n");
                  exit(0);
            }
            // listening for connections
            if(listen(data_sock,1)<0){
                  perror("Listening failed!\n");
                  close(data_sock);
                  exit(0);
            }

            // Send PORT command
            if(send(sock, portCommand, sizeof(portCommand), 0) < 0){
               printf("Error sending command to server\n");
               exit(1);
            }
            //Receive server response
            if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
               printf("Error receiving server response\n");
               exit(1);
            }
            printf("%s", server_response);

            // check server response
            if (strncmp(server_response, "200", 3) != 0){
               continue;
            }

            char* client_command = strtok(command, " ");
            char* client_argument = strtok(NULL, "");

            if(strcmp(client_command, "RETR") == 0){
               char userInput[CLIENT_INPUT_SIZE];
               sprintf(userInput, "%s %s", client_command, client_argument);
               // Send command to server
               if(send(sock, userInput, sizeof(userInput), 0) < 0){
                  printf("Error sending command to server\n");
                  exit(1);
               }
               // accept
               int server_sock = accept(data_sock,(struct sockaddr *) &server_addr,(socklen_t *)&addrlen);
               if (server_sock<0){
                     close(data_sock);
                     exit(0);
               }
               //Receive server response
               if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
                  printf("Error receiving server response\n");
                  exit(1);
               }
               printf("%s", server_response);
               // check server response
               if (strncmp(server_response, "150", 3) != 0){
                  continue;
               }
               retrieveFile(client_argument, server_sock, data_sock);
            }
            else if(strcmp(client_command, "STOR") == 0){
               char userInput[CLIENT_INPUT_SIZE];
               if (client_argument != NULL){
                  FILE *file = fopen(client_argument, "rb");
                  if(file == NULL){
                     printf("550 No such file or directory\n");
                     fclose(file);
                     // close(server_sock);
                     client_argument = "";
                     sprintf(userInput, "%s %s", client_command, client_argument);
                     // Send command to server
                     if(send(sock, userInput, sizeof(userInput), 0) < 0){
                        printf("Error sending command to server\n");
                        exit(1);
                     }
                     //Receive server response
                     if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
                        printf("Error receiving server response\n");
                        exit(1);
                     }
                     continue;
                  }
                  else
                  {
                     printf("150 File status okay; about to open data connection.\n");
                     // Send command to server
                     sprintf(userInput, "%s %s", client_command, client_argument);
                     if (send(sock, userInput, sizeof(userInput), 0) < 0)
                     {
                        printf("Error sending command to server\n");
                        exit(1);
                     }
                     // accept
                     int server_sock = accept(data_sock,(struct sockaddr *) &server_addr,(socklen_t *)&addrlen);
                     if (server_sock<0){
                           close(data_sock);
                           exit(0);
                     }
                     // Send file to server
                     sendFile(client_argument, server_sock, data_sock, file);
                  }
               }else{
                  // Send command to server
                  if(send(sock, command, sizeof(command), 0) < 0){
                     printf("Error sending command to server\n");
                     exit(1);
                  }
                  //Receive server response
                  if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
                     printf("Error receiving server response\n");
                     exit(1);
                  }
                  printf("%s", server_response);
                  // check server response
                  if (strncmp(server_response, "150", 3) != 0){
                     continue;
                  }
               }                 
            }
            else if(strcmp(client_command, "LIST") == 0){
               if(client_argument != NULL){
                  char userInput[CLIENT_INPUT_SIZE];
                  sprintf(userInput, "%s %s", client_command, client_argument);
                  // Send command to server
                  if(send(sock, userInput, sizeof(userInput), 0) < 0){
                     printf("Error sending command to server\n");
                     exit(1);
                  }
               }
               else{
                  // Send command to server
                  if(send(sock, command, sizeof(command), 0) < 0){
                     printf("Error sending command to server\n");
                     exit(1);
                  }
               }
               // accept
               int server_sock = accept(data_sock,(struct sockaddr *) &server_addr,(socklen_t *)&addrlen);
               if (server_sock<0){
                     close(data_sock);
                     exit(0);
               }
               //Receive server response
               if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
                  printf("Error receiving server response\n");
                  exit(1);
               }
               printf("%s", server_response);
               // check server response
               if (strncmp(server_response, "150", 3) != 0){
                  continue;
               }
               //List files in server
               listFile(client_argument, server_sock, data_sock);
            }
            // // Check if there is still data to be received
            // if(recv(sock, &server_response, sizeof(server_response), MSG_DONTWAIT) > 0){
            //    printf("%s", server_response);
            // }
            // else{
            //    printf("226 Transfer completed.\n");
            // }

            //Receive transfer complete message
            if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
               printf("Error receiving server response\n");
               exit(1);
            }
            printf("%s", server_response);
         }
         else if(strncmp(command, "PORT", 4) == 0){
            printf("202 Command not implemented.\n");
         }
         else{
            //Send command to server
            if(send(sock, command, sizeof(command), 0) < 0){
               printf("Error sending command to server\n");
               exit(1);
            }
            //Receive server response
            if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
               printf("Error receiving server response\n");
               exit(1);
            }
            printf("%s", server_response);
         }
      }
      if(strcmp(command, "QUIT") == 0){
         break;
      }
   }
   return 0;
}


void execute_locally(char* input_command){
   char* client_command = strtok(input_command, " ");
   char* client_argument = strtok(NULL, "");

   if (strcmp(client_command, "!LIST") == 0){
      if (client_argument != NULL){
         printf("Syntax error in parameters or arguments.\n");
      }
      else system("ls");
   }
   else if (strcmp(client_command, "!CWD") == 0){
      if (client_argument == NULL){
         printf("Syntax error in parameters or arguments.\n");
      }
      else{
         char cwd[CLIENT_INPUT_SIZE];
         sprintf(cwd, "./%s", client_argument);
         if (chdir(cwd) != 0){
               printf("Couldn't change working directory.\n");
         }
      }
   }
   else if (strcmp(client_command, "!PWD") == 0){
      if (client_argument != NULL){
         printf("Syntax error in parameters or arguments.\n");
      }
      else system("pwd");
   }
   else{
      printf("Command not implemeneted.\n");
   }
}

void retrieveFile(char* filename, int server_sock, int data_sock){
   //Open file
   int randNumber = rand() % 1000;
   char *tempFileName = malloc(100);
   sprintf(tempFileName, "%d%s", randNumber, filename);
   FILE* file = fopen(tempFileName, "wb");
   if(file == NULL){
      printf("Error opening file to write\n");
      fclose(file);
      close(server_sock);
      close(data_sock);
      return;
   }

   //Receive file
   char file_buffer[FILE_BUFFER_SIZE];
   bzero(file_buffer, sizeof(file_buffer));
   int bytes_read;
   while((bytes_read = recv(server_sock, file_buffer, FILE_BUFFER_SIZE, 0)) > 0){
      fwrite(file_buffer, 1, bytes_read, file);
      bzero(file_buffer, sizeof(file_buffer));
   }
   fclose(file);
   rename(tempFileName, filename);
   close(data_sock);
   close(server_sock);
   return;
}

void sendFile(char* filename, int server_sock, int data_sock, FILE* file){
   //Send file
   char file_buffer[FILE_BUFFER_SIZE];
   int bytes_read;
   bzero(file_buffer, sizeof(file_buffer));
   while((bytes_read = fread(file_buffer, 1, FILE_BUFFER_SIZE, file)) > 0){
      send(server_sock, file_buffer, bytes_read, 0);
      bzero(file_buffer, sizeof(file_buffer));
   }
   fclose(file);
   close(server_sock);
   close(data_sock);
}

void listFile(char* filename, int server_sock, int data_sock){
   //Receive file
   char file_buffer[FILE_BUFFER_SIZE];
   int bytes_read;
   bzero(file_buffer, sizeof(file_buffer));
   while((bytes_read = recv(server_sock, file_buffer, FILE_BUFFER_SIZE, 0)) > 0){
      printf("%s", file_buffer);
      bzero(file_buffer, sizeof(file_buffer));
   }
   close(server_sock);
   close(data_sock);
}