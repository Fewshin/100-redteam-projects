#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

//#define PORT 69

int main (int argc, char* argv[]) {

  int clientSocket;
  int status;
  ssize_t readVar;

  struct sockaddr_un hostAddr;
  int outgoingLength = atoi(argv[1]);
  char* outgoing = (char*) malloc(outgoingLength);
  char* incoming = (char*) malloc(2048);

  memcpy(outgoing, "test\n", 4);

  hostAddr.sun_family = AF_UNIX;
  strcpy(hostAddr.sun_path, "/mnt/e/SpookyHax/100RedteamProjects/100-redteam-projects/mySolutions/0/link");

  if ((clientSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("client socket failed to make");
    exit(EXIT_FAILURE);
  }

  if ((status = connect(clientSocket, (struct sockaddr*)&hostAddr, sizeof(hostAddr))) < 0) {
    perror("client failed to connect to host");
    exit(EXIT_FAILURE);
  }

  printf("This is the message being sent to the host: %s\n", outgoing);
  send(clientSocket, outgoing, strlen(outgoing), 0);
  free(outgoing);
  printf("The message was sent to the host.\n");

  readVar = read(clientSocket, incoming, 2047);
  printf("The following message was recieved from the host: %s\n", incoming);
  free(incoming);

  close(clientSocket);

  return 0;
}