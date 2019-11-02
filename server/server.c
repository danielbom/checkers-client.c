
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Alias to increase readability
typedef struct sockaddr_in SocketAddrIn;
typedef struct sockaddr SocketAddr;

char masterBuffer[1025];

int masterID = 0;
SocketAddrIn masterAddress;
int masterAddressLength = 0;

int masterFamily = AF_INET;           // AF_INET: Address family Internet Protocol v4 addresses
int masterType = SOCK_STREAM;         // SOCK_STREAM: Connection-based TCP
int masterMaskInAddress = INADDR_ANY; // Address to accept any incoming messages.
int masterPort = 12345;

int limitOfClients = 50;
int clientSockets[50] = {0};

int error = 0;
void rejectCriticalError(char* message, int isCriticalError) {
  if (isCriticalError) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}

void loop() {
  
}

void initMasterSocket() {
  int opt = 1;

  masterID = socket(masterFamily, masterType, 0);
  rejectCriticalError("(socket) Failed to create master socket", masterID == -1);

  error = setsockopt(masterID, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
  rejectCriticalError("(setsockopt) Failed to update master socket to allow multiples connections", error == -1);

  // Setup master socket
  masterAddress.sin_family = masterFamily;
  masterAddress.sin_addr.s_addr = masterMaskInAddress;
  masterAddress.sin_port = masterPort;

  error = bind(masterID, (SocketAddr*) &masterAddress, sizeof(masterAddress));
  rejectCriticalError("(bind) Failed to bind master socket in give address", error == -1);

  error = listen(masterID, 3);
  rejectCriticalError("(listen) Failed to prepare to accept connections", error == -1);

  printf(">>> Sizeof(masterAddress): %ld\n", sizeof(masterAddress));
  printf(">>> Server socket listen on port '%d'\n", masterPort);
}

int main() {
  initMasterSocket();

  while (1) {
    loop();
  }

  return 0;
}