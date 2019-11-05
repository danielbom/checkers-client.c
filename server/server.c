#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/time.h>
#include <errno.h>

#include "utils.server.c"


SocketAddrIn masterAddress;
int masterAddressLength = 0;

int masterID = 0;

int masterFamily = AF_INET;           // AF_INET: Address family Internet Protocol v4 addresses
int masterType = SOCK_STREAM;         // SOCK_STREAM: Connection-based TCP
int masterMaskInAddress = INADDR_ANY; // Address to accept any incoming messages.
int masterPort = 12345;

int limitOfClients = 50;
int filledClients = 0;
int clientListSockets[50] = {0};
int reservedRoomClients[50] = {0};
fd_set systemSetDescriptor;

void loop() {
  char masterBuffer[1025];
  int i, maxID = masterID;

  while (1) {
    FD_ZERO(&systemSetDescriptor);

    FD_SET(masterID, &systemSetDescriptor);

    for (i = 0; i < limitOfClients; i++) {
      int sd = clientListSockets[i]; // SocketDescriptor
      if (sd > 0)
        FD_SET(sd, &systemSetDescriptor);
      if (sd > maxID)
        maxID = sd;
    }

    int id = select(maxID + 1, &systemSetDescriptor, NULL, NULL, NULL);
    printf("ID: %d\n", id);

    if (id < -1 && errno != EINTR)
      printf("(select) Fail to select an ID of set of sockets\n");
    
    if (FD_ISSET(masterID, &systemSetDescriptor)) {
      if (filledClients == limitOfClients) {
        printf("Server is full\n");
      } else {
        int newSocket = accept(masterID, NULL, NULL);
        rejectCriticalError("(accept) Error when master socket accept new connection\n", newSocket == -1);

        printf("New connection\n");
        showHostInfos(newSocket);

        for (i = 0; i < limitOfClients; i++) {
          if (clientListSockets[i] == 0) {
            clientListSockets[i] = newSocket;
            filledClients++;
            break;
          }
        }
      }
    }

    for (i = 0; i < limitOfClients; i++) {
      int sd = clientListSockets[i];
      if (FD_ISSET(sd, &systemSetDescriptor)) {
        int numberOfBytes = read(sd, masterBuffer, 1024);
        if (numberOfBytes == 0) {
          printf("Host disconnected\n");
          showHostInfos(sd);
          clientListSockets[i] = 0;
          break;
        } else {
          masterBuffer[numberOfBytes] = 0;
          showHostInfos(sd);
          printf("Received: '%s'\n", masterBuffer);
          send(sd, masterBuffer, numberOfBytes, 0);
          break;
        }
      }
    }
  }
}

void initMasterSocket() {
  int opt = 1, error;

  masterID = socket(masterFamily, masterType, 0);
  rejectCriticalError("(socket) Failed to create master socket", masterID == -1);

  error = setsockopt(masterID, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
  rejectCriticalError("(setsockopt) Failed to update master socket to allow multiples connections", error == -1);

  // Setup master socket
  masterAddress.sin_family = masterFamily;
  masterAddress.sin_addr.s_addr = masterMaskInAddress;
  masterAddress.sin_port = htons(masterPort);

  error = bind(masterID, (SocketAddr*) &masterAddress, sizeof(masterAddress));
  rejectCriticalError("(bind) Failed to bind master socket in give address", error == -1);

  error = listen(masterID, 3);
  rejectCriticalError("(listen) Failed to prepare to accept connections", error == -1);

  printf(">>> Sizeof(masterAddress): %ld\n", sizeof(masterAddress));
  printf(">>> Server socket listen on port '%d'\n", masterPort);
}

int main() {
  initMasterSocket();

  loop();

  return 0;
}