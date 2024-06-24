#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define PORT 8080

int main (int argc, char * argv[]) {
  int clientSocket;
  int readMessage;
  int maxMessageLength = 2047;
  bool kill = false;
  int connectionAttempt = 0;
  int handshake = 0;
  int clientCount = 2;
  
  //char * userServerAddress = "penis";

  struct sockaddr_in hostAddr;
  socklen_t hostAddLen = sizeof(hostAddr);
  // char * outgoing = malloc(sizeof(char) * maxMessageLength + 1);
  // char * incoming = malloc(sizeof(char) * maxMessageLength + 1); //redudant?
  char * message = malloc(sizeof(char) * maxMessageLength + 1);

  hostAddr.sin_family = AF_INET;
  hostAddr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &hostAddr.sin_addr) <= 0) {
    perror("Invalid address/ Address not supported \n");
    exit(EXIT_FAILURE);
  }
  //printf("Input server address\n");
  //scanf("%s", )

  if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) { //creating client socket
    perror("client socket failed to make");
    exit(EXIT_FAILURE);
  }

  if ((handshake = connect(clientSocket, (struct sockaddr*)&hostAddr, sizeof(hostAddr))) < 0) {
    perror("client failed to connect to host");
    exit(EXIT_FAILURE);
  }

  fd_set clientList;
  fd_set currentList;
  FD_ZERO(&clientList);
  FD_SET(clientSocket, &clientList);
  FD_SET(fileno(stdin), &clientList);

  struct timeval waitTime;
  waitTime.tv_sec = 30;
  waitTime.tv_usec = 0;

  while(!kill) {
    FD_ZERO(&currentList);
    memcpy(&currentList, &clientList, sizeof(clientList));

    connectionAttempt = select(20, &currentList, NULL, NULL, /*waitTime*/NULL);

    if (FD_ISSET(clientSocket, &currentList)) {
      //printf("incoming communications\n");
      if ((readMessage = read(clientSocket, message, maxMessageLength + 1)) == 0) {
        printf("Server offline\n");
        kill = true;
      } else {
        printf("%s\n", message);
      }
    } else if (FD_ISSET(fileno(stdin), &currentList)) {
      if((readMessage = read(fileno(stdin), message, maxMessageLength + 1)) == 0) {
        //Not sure what this case is
      } else {
        //fflush(stdin);
        message[readMessage] = '\0';
        message = realloc(message, readMessage+1);
        send(clientSocket, message, strlen(message), 0);
        message = realloc(message, maxMessageLength + 1);
        //printf("message sent\n");
      }
    } else {
      perror("Invalid connection attempt?");
      exit(EXIT_FAILURE);
    }
  }

  close(clientSocket);

  // free(outgoing);
  // free(incoming);
  free(message);
  return 0;
}