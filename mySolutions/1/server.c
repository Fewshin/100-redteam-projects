#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/select.h>
//#include <sys/time.h>

#define PORT 8080

int main (int argc, char * argv[]) {
  int serverSocket;
  int connectionLimit = 16;
  int * connectionSocket = malloc(sizeof(int) * connectionLimit);
  int readMessage;
  int yes = 1;
  int maxMessageLength = 2047;
  bool kill = false;
  int clientCount = 1;
  int connectionAttempt = 0;
  
  struct sockaddr_in hostAddress;
  socklen_t hostAddLen = sizeof(hostAddress);
  char * outgoing = malloc(sizeof(char) * maxMessageLength + 1); // wtf lmao

  hostAddress.sin_family = AF_INET;
  hostAddress.sin_addr.s_addr = INADDR_ANY;
  hostAddress.sin_port = htons(PORT);

  for (int i = 0; i < connectionLimit; i++) {
    connectionSocket[i] = 0;
  }

  if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) { //creating host socket
    perror("listening socket failed to make");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) < 0) { //setting options for the host socket i.e. allowing multiple sockets on the same address and port
    perror("failed setting socket options"); 
    exit(EXIT_FAILURE);
  }

  if (bind(serverSocket, (struct sockaddr*)&hostAddress, sizeof(hostAddress)) < 0) { //binding to the port so the socket can do something
    perror("failed to bind to port");
    exit(EXIT_FAILURE);
  }

  if ((listen(serverSocket, 16)) < 0) { //telling the socket to start listening for incomming connections
    perror("failed to listen");
    exit(EXIT_FAILURE);
  } 

  fd_set clientList;
  fd_set currentList;
  FD_ZERO(&clientList);
  //FD_ZERO(&currentList);
  FD_SET(serverSocket, &clientList);

  struct timeval waitTime;
  waitTime.tv_sec = 30;
  waitTime.tv_usec = 0;

  while (!kill) {
    // for (int i = 0; i < connectionLimit; i++) {
    //   if (connectionSocket[i] > 0) {
    //     FD_SET(connectionSocket[i], &clientList);
    //     //clientCount++;
    //   }
    // }
    FD_ZERO(&currentList);
    //FD_COPY(&clientList, &currentList);
    memcpy(&currentList, &clientList, sizeof(clientList));
    //currentList = clientList;

    connectionAttempt = select(connectionLimit + 1, &currentList, NULL, NULL, /*waitTime*/NULL);

    if(FD_ISSET(serverSocket, &currentList)) {
      if ((connectionSocket[clientCount+1] = accept(serverSocket, (struct sockaddr*)&hostAddress, &hostAddLen)) < 0) { 
        perror("failed to connect to new client");
        exit(EXIT_FAILURE);
      } else {
        clientCount++;
        FD_SET(connectionSocket[clientCount], &clientList);
        printf("Client %i connected successfully\n", connectionSocket[clientCount]);
      }

      //TODO: Communicate to other clients a new client connected to the server.

    } else {
      for (int i = 0; i < connectionLimit; i++) {
        if (FD_ISSET(connectionSocket[i], &currentList)) {
          if ((readMessage = read(connectionSocket[i], outgoing, maxMessageLength + 1)) == 0) {
            int disconnectedPeer = getpeername(connectionSocket[i], (struct sockaddr*)&hostAddress, &hostAddLen);
            printf("Client %i Disconnected.\n", connectionSocket[i]);
            FD_CLR(connectionSocket[i], &clientList);
            close(connectionSocket[i]);
            connectionSocket[i] = 0;
            int * newList = malloc(sizeof(int) * connectionLimit);
            int * oldList = connectionSocket;
            int validClient = 0;
            for (int j = 0; j < clientCount; j++) {
              if (connectionSocket[j] > 0) {
                newList[validClient] = connectionSocket[j];
                validClient++;
              }
            }
            clientCount--;
            connectionSocket = newList;
            free(oldList);
            for (int j = clientCount; j < connectionLimit; j++) {
              connectionSocket[j] = 0;
            }
          } else {
            outgoing[readMessage] = '\0';
            outgoing = realloc(outgoing, readMessage+1); //lmao don't dump memory that doesn't contain the message
            printf("Message Recieved: %s", outgoing);
            for (int j = 0; j < connectionLimit; j++) {
              if ((send(connectionSocket[j], outgoing, strlen(outgoing), 0)) < 0) {
                printf("message failed to send to client: %i\n", connectionSocket[j]);
              } else {
                printf("message sent to client: %i\n", connectionSocket[j]);
              }
            }
            outgoing = realloc(outgoing, maxMessageLength+1); //Re-expand buffer
          }
        }
      }
    }
    //printf("listening cycle ended.\n");
  }

  close(serverSocket);

  free(outgoing);
  free(connectionSocket);
  return 0;
}