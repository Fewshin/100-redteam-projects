#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

//#define PORT 69

int main (int argc, char* argv[]) {
  
  const int yes = 1;

  int listeningSocket;
  int clientSocket;
  ssize_t readVar;

  struct sockaddr_un hostAddress;
  socklen_t hostAddLen = sizeof(hostAddress);
  int outgoingLength = atoi(argv[1]);
  char* outgoing = (char*) malloc(outgoingLength);
  char* incoming = (char*) malloc(2048);

  memcpy(outgoing, "test\n", 4);

  hostAddress.sun_family = AF_UNIX;
  strcpy(hostAddress.sun_path, "/mnt/e/SpookyHax/100RedteamProjects/100-redteam-projects/mySolutions/0/link");

  if ((listeningSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) { //creating host socket
    perror("listening socket failed to make");
    exit(EXIT_FAILURE);
  }

  // if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) < 0) { //setting options for the host socket i.e. allowing multiple sockets on the same address and port
  //    perror("failed setting socket options"); 
  //    exit(EXIT_FAILURE);
  //  }
  unlink(hostAddress.sun_path);

  if (bind(listeningSocket, (struct sockaddr*)&hostAddress, sizeof(hostAddress)) < 0) { //binding to the port so the socket can do something
    perror("failed to bind to port");
    exit(EXIT_FAILURE);
  }

  if ((listen(listeningSocket, 16)) < 0) { //telling the socket to start listening for incomming connections
    perror("failed to listen");
    exit(EXIT_FAILURE);
  } 

  if((clientSocket = accept(listeningSocket, NULL, 0)) < 0) {
    perror("client socket failed to make");
    exit(EXIT_FAILURE);
  }

  readVar = read(clientSocket, incoming, 2047);
  printf("This is the message being recieved at Host: %s\n", incoming);
  free(incoming);

  printf("This is the message being sent from Host: %s\n", outgoing);
  send(clientSocket, outgoing, strlen(outgoing), 0);
  printf("Message Sent from Host.\n");
  free(outgoing);

  close(clientSocket);
  close(listeningSocket);

  return 0;
}