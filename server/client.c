#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <pthread.h>

#include "utils.server.c"

#define BUFFER_CLIENT_SIZE 512

int socketPort = 12345;
int socketClient;
int runningClient = 1;

enum {
  OP_LIST = 1,
  OP_CONNECT = 2,
  OP_CREATE_ROOM = 3,
  OP_SEND_MESSAGE = 4,
  OP_EXIT = 5
};

void* sender(void* arg) {
  char buffer[BUFFER_CLIENT_SIZE + 1] = {0};
  while (runningClient) {
    setbuf(stdin , NULL);
    scanf("%[^\n]", buffer);
    send(socketClient, buffer, strlen(buffer), 0);
    printf("Send: %s\n", buffer);
    buffer[0] = 0;
  }
}

void senderRunner(pthread_t* thread) {
  pthread_create(thread, NULL, sender, NULL);
}

void* receiver(void* callback) {
  char buffer[BUFFER_CLIENT_SIZE + 1] = {0};
  while (runningClient) {
    int numberOfBytes = read(socketClient, buffer, BUFFER_CLIENT_SIZE);
    if (numberOfBytes == 0) {
      printf("Server disconnected\n");
      runningClient = 0;
    } else {
      buffer[numberOfBytes] = 0;
      printf("Received: '%s'\n", buffer);
    }
  }
}

void receiverRunner(pthread_t* thread, void (*callback)(void*)) {
  pthread_create(thread, NULL, receiver, callback);
}

void initClientSocket() {
  int error;

  socketClient = socket(AF_INET, SOCK_STREAM, 0);
  rejectCriticalError("(socket) Failed to create client socket", socketClient == -1);

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(socketPort);
  error = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
  rejectCriticalError("(inet_pton) Invalid address", error == -1);

  error = connect(socketClient, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  rejectCriticalError("(connect) Failed to connect with the server", error == -1);
}

// Sets
void setErrorOnPacket(char* packet, int error) {
  memcpy(packet, &error, sizeof(int));
}
void setOperationOnPacket(char* packet, int op) {
  memcpy(packet + sizeof(int), &op, sizeof(int));
}
void setUsernameOnPacket(char* packet, char* user) {
  strncat(packet + (sizeof(int) * 2), user, 63);
}
void setPasswordOnPacket(char* packet, char* pass) {
  strncat(packet + (sizeof(int) * 2) + 64, pass, 63);
}
void setNameRoomOnPacket(char* packet, char* room) {
  strncat(packet + (sizeof(int) * 2) + (64 * 2), room, 63);
}

// Gets
int getErrorOfPacket(char* packet) {
  int error;
  memcpy(&error, packet, sizeof(int));
  return error;
}
int getOperationOfPacket(char* packet) {
  int op;
  memcpy(&op, packet + sizeof(int), sizeof(int));
  return op;
}

// Create
char* createPacketToListRoomsByClient() {
  char *buffer = calloc(16, sizeof(char));
  setOperationOnPacket(buffer, OP_LIST);
  return buffer;
}
char* createPacketToConnectByClient(char* name, char* password, char* roomName) {
  char *buffer = calloc(256, sizeof(char));
  setUsernameOnPacket(buffer, name);
  setPasswordOnPacket(buffer, password);
  setNameRoomOnPacket(buffer, roomName);
  setOperationOnPacket(buffer, OP_CONNECT);
  return buffer;
}
char* createPacketToCreateRoomByClient(char* name, char* password, char* roomName, int numberOfUsers) {
  char *buffer = createPacketToConnectByClient(name, password, roomName);
  // Add number of users in room
  memcpy(buffer + (sizeof(int) * 2) + (64 * 3), &numberOfUsers, sizeof(int));
  setOperationOnPacket(buffer, OP_CREATE_ROOM);
  return buffer;
}
void* createPacketToSendMessageByClient(char* name, char* roomName, char* message) {
  char* buffer = calloc(512, sizeof(char));
  int shift = 8;

  setUsernameOnPacket(buffer, name);
  shift += 64;

  // Add classroom
  strncat(buffer + shift, roomName, 63);
  shift += 64;

  // Add message
  strncat(buffer + shift, message, 255);

  setOperationOnPacket(buffer, OP_SEND_MESSAGE);
  return buffer;
}
void* createPacketToExitByClient(char *name, char *password) {
  char* buffer = calloc(256, sizeof(char));
  setUsernameOnPacket(buffer, name);
  setPasswordOnPacket(buffer, password);
  setOperationOnPacket(buffer, OP_EXIT);
  return buffer;
}

// Read
void readPacketToListRoomsByClient(char* packet) {
  int shift = sizeof(int) * 2;
  int error = getErrorOfPacket(packet);
  int op = getOperationOfPacket(packet);

  printf("Error: '%d', Operation: '%d'\n", error, op);
}
void readPacketToConnectByClient(char* packet) {
  int shift = sizeof(int) * 2;
  int error = getErrorOfPacket(packet);
  int op = getOperationOfPacket(packet);
  char* name = packet + shift;
  shift += 64;
  char* password = packet + shift;
  shift += 64;
  char* roomName = packet + shift;

  printf("Error: '%d', Operation: '%d', ", error, op);
  printf("Username: '%s', Password: '%s', Room name: '%s'\n", name, password, roomName);
}
void readPacketToCreateRoomByClient(char* packet) {
  int shift = sizeof(int) * 2;
  int error = getErrorOfPacket(packet);
  int op = getOperationOfPacket(packet);
  char* name = packet + shift;
  shift += 64;
  char* password = packet + shift;
  shift += 64;
  char* roomName = packet + shift;
  int numberOfUsers;
  memcpy(&numberOfUsers, packet + shift, 4);

  printf("Error: '%d', Operation: '%d', ", error, op);
  printf("Username: '%s', Password: '%s', Room name: '%s', Number of users: '%d'\n", name, password, roomName, numberOfUsers);
}
void readPacketToSendMessageByClient(char* packet) {
  int shift = sizeof(int) * 2;
  int error = getErrorOfPacket(packet);
  int op = getOperationOfPacket(packet);
  char* name = packet + shift;
  shift += 64;
  char* roomName = packet + shift;
  shift += 64;
  char* message = packet + shift;

  printf("Error: '%d', Operation: '%d', ", error, op);
  printf("Username: '%s', Room name: '%s', Message: '%s'\n", name, roomName, message);
}
void readPacketToExitByClient(char *packet) {
  int shift = sizeof(int) * 2;
  int error = getErrorOfPacket(packet);
  int op = getOperationOfPacket(packet);
  char* name = packet + shift;
  shift += 64;
  char* password = packet + shift;

  printf("Error: '%d', Operation: '%d', ", error, op);
  printf("Username: '%s', Password: '%s'\n", name, password);
}

void testOnePacket(char* packet) {
  if (!getErrorOfPacket(packet)) {
    int op = getOperationOfPacket(packet);
    switch(op) {
      case OP_LIST:
      readPacketToListRoomsByClient(packet);
      break;
      case OP_CONNECT:
      readPacketToConnectByClient(packet);
      break;
      case OP_CREATE_ROOM:
      readPacketToCreateRoomByClient(packet);
      break;
      case OP_EXIT:
      readPacketToExitByClient(packet);
      break;
    }
  }
  free(packet);
}

void testPackets() {
  testOnePacket(createPacketToListRoomsByClient());
  testOnePacket(createPacketToConnectByClient("user-123", "1234", "-room-"));
  testOnePacket(createPacketToCreateRoomByClient("user-123", "1234", "-room-", 2));
  testOnePacket(createPacketToSendMessageByClient("user-123", "1234", "Hello world!"));
  testOnePacket(createPacketToExitByClient("user-123", "1234"));
}

int main(int argc, char const *argv[]) {
  testPackets();
  /*
  int valread;

  initClientSocket();

  pthread_t thread;
  senderRunner(&thread);
  receiverRunner(&thread, NULL);

  printf("Loop...\n");
  while (runningClient) {}
  */
  return 0;
}
