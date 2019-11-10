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

#define BUFFER_SERVER_SIZE 1024

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


// Read
void readPacketToListRoomsOfClient(int sd, char* packet) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf("\n");
}
void readPacketToConnectOfClient(int sd, char* packet) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  char* username = ByteBufferGetString(packet, &shift);
  char* password = ByteBufferGetString(packet, &shift);
  char* roomName = ByteBufferGetString(packet, &shift);
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', Password: '%s', Room name: '%s'\n", username, password, roomName);
}
void readPacketToCreateRoomOfClient(int sd, char* packet) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  char* username = ByteBufferGetString(packet, &shift);
  char* password = ByteBufferGetString(packet, &shift);
  char* roomName = ByteBufferGetString(packet, &shift);
  int numberOfUsers = ByteBufferGetInt(packet, &shift);
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', Password: '%s', Room name: '%s', Number of users: '%d'\n", username, password, roomName, numberOfUsers);
}
void readPacketToSendMessageOfClient(int sd, char* packet) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  char* username = ByteBufferGetString(packet, &shift);
  char* roomName = ByteBufferGetString(packet, &shift);
  char* message = ByteBufferGetString(packet, &shift);
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', RoomName: '%s', Message: '%s'\n", username, roomName, message);
}
void readPacketToExitOfClient(int sd, char* packet) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  char* username = ByteBufferGetString(packet, &shift);
  char* password = ByteBufferGetString(packet, &shift);
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', Password: '%s'\n", username, password);
}
void readPacketOfClient(int sd, char* packet) {
  showHostInfos(sd);
  int error = getErrorOfPacket(packet);
  if (!error) {
    int op = getOperationOfPacket(packet);
    switch(op) {
      case OP_LIST:
      readPacketToListRoomsOfClient(sd, packet);
      break;
      case OP_CONNECT:
      readPacketToConnectOfClient(sd, packet);
      break;
      case OP_CREATE_ROOM:
      readPacketToCreateRoomOfClient(sd, packet);
      break;
      case OP_SEND_MESSAGE:
      readPacketToSendMessageOfClient(sd, packet);
      break;
      case OP_EXIT:
      readPacketToExitOfClient(sd, packet);
      break;
    }
  } else {
    checkErrorOfServer(error);
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
void loop() {
  char masterBuffer[BUFFER_SERVER_SIZE + 1];
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
        int numberOfBytes = read(sd, masterBuffer, BUFFER_SERVER_SIZE);
        if (numberOfBytes == 0) {
          printf("Host disconnected\n");
          showHostInfos(sd);
          clientListSockets[i] = 0;
          break;
        } else { // Broadcast
          printf("Broadcast %d\n", numberOfBytes);
          masterBuffer[numberOfBytes] = 0;
          printf("Received: '%s'\n", masterBuffer);
          readPacketOfClient(sd, masterBuffer);
          break;
        }
      }
    }
  }
}

int main() {
  initMasterSocket();
  loop();
  return 0;
}