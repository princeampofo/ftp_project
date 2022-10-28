#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define IP_ADDRESS "127.0.0.1"
#define CLIENT_INPUT_SIZE 100
#define SERVER_RESPONSE_SIZE 1500
#define FILE_BUFFER_SIZE 1024

void execute_commandWhenNotLoggedIn(int sock);
void execute_badusername(int sock);
void execute_goodusername(int sock);
void execute_badpassword(int sock);
void execute_goodpassword(int sock);
void execute_unImplementedCommand(int sock);
void sendingPort(int sock, int PORT, char* command);
void execute_badpwd(int sock);
void execute_pwd(int sock);
void execute_badcwd(int sock);
void execute_cwd(int sock);
void execute_quit(int sock);
void listFile(char* filename, int server_sock, int data_sock, int sock);
void sendFile(char* filename, int server_sock, int data_sock, FILE* file,int sock);
void retrieveFile(char* filename, int server_sock, int data_sock, int sock);

int main(int argc, char *argv[])
{
   // Program to test ftp server
   system("make clean");
   system("make");
   chdir("./server");

   // Make ftp_server executable by root
   // system("sudo chmod 4755 ftp_server");

   // Start ftp_server
   system("sudo ./ftp_server &");

   // Wait for ftp_server to start
   sleep(1);

   // Come back to root directory
   chdir("..");
   
   // Work on client for testing 
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

   // // Send user name
   // execute_commandWhenNotLoggedIn(sock);
   // execute_badusername(sock);
   execute_goodusername(sock);
   // execute_badpassword(sock);
   execute_goodpassword(sock);
   // execute_unImplementedCommand(sock);

   char command[CLIENT_INPUT_SIZE];

   PORT += 1;
   bzero(command, CLIENT_INPUT_SIZE);
   sprintf(command, "%s", "LIST bs");
   printf("Testing invalid LIST command with wrong arguments\n");
   sendingPort(sock, PORT, command);

   PORT += 1;
   bzero(command, CLIENT_INPUT_SIZE);
   sprintf(command, "%s", "LIST");
   printf("Testing valid LIST command\n");
   sendingPort(sock, PORT, command);

   PORT += 1;
   bzero(command, CLIENT_INPUT_SIZE);
   sprintf(command, "%s", "RETR bs.txt");
   printf("Testing invalid RETR command with wrong filename\n");
   sendingPort(sock, PORT, command);

   PORT += 1;
   bzero(command, CLIENT_INPUT_SIZE);
   sprintf(command, "%s", "RETR");
   printf("Testing invalid RETR command with no filename\n");
   sendingPort(sock, PORT, command);

   PORT += 1;
   bzero(command, CLIENT_INPUT_SIZE);
   sprintf(command, "%s", "RETR users.txt");
   printf("Testing valid RETR command\n");
   sendingPort(sock, PORT, command);

   PORT += 1;
   bzero(command, CLIENT_INPUT_SIZE);
   sprintf(command, "%s", "STOR xvt.txt");
   printf("Testing invalid STOR command with wrong filename\n");
   sendingPort(sock, PORT, command);

   PORT += 1;
   bzero(command, CLIENT_INPUT_SIZE);
   sprintf(command, "%s", "STOR");
   printf("Testing invalid STOR command with no filename\n");
   sendingPort(sock, PORT, command);

   PORT += 1;
   bzero(command, CLIENT_INPUT_SIZE);
   sprintf(command, "%s", "STOR test.txt");
   printf("Testing valid STOR command\n");
   sendingPort(sock, PORT, command);

   execute_badpwd(sock);
   execute_pwd(sock);
   execute_badcwd(sock);
   execute_cwd(sock);
   execute_quit(sock);

   // Close socket
   close(sock);

   // Kill ftp_server
   system("sudo killall ftp_server");

   // Make clean
   system("make clean");
   return 0;
}

void execute_commandWhenNotLoggedIn(int sock){
   printf("Testing invalid commands when not logged in\n");
   // Send command when not logged in
   char client_input[CLIENT_INPUT_SIZE] = "LIST";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "530", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void execute_badusername(int sock){
   printf("Testing invalid username\n");
   // Send wrong username
   char client_input[CLIENT_INPUT_SIZE] = "USER badusername";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "530", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void execute_goodusername(int sock){
   printf("Testing valid username\n");
   // Send good username
   char client_input[CLIENT_INPUT_SIZE] = "USER bob";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "331", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void execute_badpassword(int sock){
   printf("Testing invalid password\n");
   // Send bad password
   char client_input[CLIENT_INPUT_SIZE] = "PASS badpassword";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "530", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void execute_goodpassword(int sock){
   printf("Testing valid password\n");
   // Send good password
   char client_input[CLIENT_INPUT_SIZE] = "PASS donuts";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "230", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void execute_unImplementedCommand(int sock){
   printf("Testing unimplemented command\n");
   // Send unimplemented command
   char client_input[CLIENT_INPUT_SIZE] = "UNIMPLEMENTED";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "502", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void retrieveFile(char* filename, int server_sock, int data_sock, int sock){
   //Open file
   FILE* file = fopen(filename, "wb");
   if(file == NULL){
      fclose(file);
      close(server_sock);
      close(data_sock);
      printf("Test failed: Could not open file\n");
      exit(1);
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
   close(data_sock);
   close(server_sock);
   printf("Test passed: File received\n\n");

   // Get 226 response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
}

void sendFile(char* filename, int server_sock, int data_sock, FILE* file, int sock){
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
   printf("Test passed: File sent\n\n");

   // Get 226 response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
}

void listFile(char* filename, int server_sock, int data_sock, int sock){
   //Receive file
   char file_buffer[FILE_BUFFER_SIZE];
   int bytes_read;
   bzero(file_buffer, sizeof(file_buffer));
   while((bytes_read = recv(server_sock, file_buffer, FILE_BUFFER_SIZE, 0)) > 0){
      // printf("%s", file_buffer);
      bzero(file_buffer, sizeof(file_buffer));
   }
   close(server_sock);
   close(data_sock);
   printf("Test passed: File list received\n\n");
   // Get 226 response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
}

void execute_badpwd(int sock){
   printf("Testing invalid PWD command\n");
   // Send invalid PWD command
   char client_input[CLIENT_INPUT_SIZE] = "PWD bad";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "501", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void execute_pwd(int sock){
   printf("Testing valid PWD command\n");
   // Send valid PWD command
   char client_input[CLIENT_INPUT_SIZE] = "PWD";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "257", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void execute_badcwd(int sock){
   printf("Testing invalid CWD command\n");
   // Send invalid CWD command
   char client_input[CLIENT_INPUT_SIZE] = "CWD bad";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "550", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void execute_cwd(int sock){
   printf("Testing valid CWD command\n");
   // Send valid CWD command
   char client_input[CLIENT_INPUT_SIZE] = "CWD /";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "200", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void execute_quit(int sock){
   printf("Testing QUIT command\n");
   // Send quit command
   char client_input[CLIENT_INPUT_SIZE] = "QUIT";
   if(send(sock, client_input, sizeof(client_input), 0) < 0){
      printf("Error sending client input\n");
      exit(1);
   }

   // Receive server response
   char server_response[SERVER_RESPONSE_SIZE];
   if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
      printf("Error receiving server response\n");
      exit(1);
   }
   if(strncmp(server_response, "221", 3) == 0){
      printf("Test passed %s\n", server_response);
   }
   else{
      printf("Test failed %s\n", server_response);
   }
}

void sendingPort(int sock, int client_ports, char* command){
   char server_response[SERVER_RESPONSE_SIZE];
   bzero(server_response, SERVER_RESPONSE_SIZE);

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
      printf("Port command is %s\n", portCommand);
      if (send(sock, portCommand, sizeof(portCommand), 0) < 0)
      {
         printf("Error sending command to server\n");
         exit(1);
      }
      //Receive server response
      if(recv(sock, &server_response, sizeof(server_response), 0) < 0){
         printf("Error receiving server response\n");
         exit(1);
      }
      // check server response
      if (strncmp(server_response, "200", 3) != 0){
         printf("Test failed %s\n", server_response);
         return;
      }

      // accept
      int server_sock = accept(data_sock,(struct sockaddr *) &server_addr,(socklen_t *)&addrlen);
      if (server_sock<0){
            close(data_sock);
            exit(0);
      }

      char userInput[CLIENT_INPUT_SIZE];
      bzero(userInput, CLIENT_INPUT_SIZE);
      strncpy(userInput, command, strlen(command));

      char* client_command = strtok(userInput, " ");
      char* client_argument = strtok(NULL, "");

      if(strcmp(client_command, "RETR") == 0){
         char userInput[CLIENT_INPUT_SIZE];
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
         // check server response
         if (strncmp(server_response, "150", 3) != 0){
            printf("Test failed %s\n", server_response);
            return;
         }
         retrieveFile(client_argument, server_sock, data_sock, sock);
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
               printf("Test passed %s\n", server_response);
            }
            else
            {
               // Send command to server
               sprintf(userInput, "%s %s", client_command, client_argument);
               if (send(sock, userInput, sizeof(userInput), 0) < 0)
               {
                  printf("Error sending command to server\n");
                  exit(1);
               }
               // Send file to server
               sendFile(client_argument, server_sock, data_sock, file, sock);
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
            // check server response
            if (strncmp(server_response, "150", 3) != 0){
               printf("Test failed %s\n", server_response);
               return;
            }
         }                 
      }
      else if(strcmp(client_command, "LIST") == 0){
         printf("We are here\n");
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
         // check server response
         if (strncmp(server_response, "150", 3) != 0){
            printf("Test failed %s\n", server_response);
            return;
         }
         //List files in server
         listFile(client_argument, server_sock, data_sock, sock);
      }
   }
   else{
      printf("Test failed. Command unknown\n");   
   }
}