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

void *sender(void *arg) {
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
void *receiver(void *callback) {
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
void setUsernameOnPacket(char* packet, char* user) {
  strncat(packet + (sizeof(int) * 3), user, 63);
}
void setPasswordOnPacket(char* packet, char* pass) {
  strncat(packet + (sizeof(int) * 3) + 64, pass, 63);
}
void setNameRoomOnPacket(char* packet, char* room) {
  strncat(packet + (sizeof(int) * 3) + (64 * 2), room, 63);
}

// Create
char* createPacketToListRoomsByClient() {
  char *buffer = ByteBufferAllocate(16);
  setOperationOnPacket(buffer, OP_LIST);
  setTypeOnPacket(buffer, TYPE_CLIENT);
  return buffer;
}
char* createPacketToConnectByClient(char* username, char* password, char* roomName) {
  char *buffer = ByteBufferAllocate(256);
  setOperationOnPacket(buffer, OP_CONNECT);
  setTypeOnPacket(buffer, TYPE_CLIENT);
  int shift = sizeof(int) * 3;
  ByteBufferPutString(buffer, &shift, username, 64);
  ByteBufferPutString(buffer, &shift, password, 64);
  ByteBufferPutString(buffer, &shift, roomName, 64);
  return buffer;
}
char* createPacketToCreateRoomByClient(char* username, char* password, char* roomName, int numberOfUsers) {
  char *buffer = ByteBufferAllocate(256);
  setOperationOnPacket(buffer, OP_CREATE_ROOM);
  setTypeOnPacket(buffer, TYPE_CLIENT);
  int shift = sizeof(int) * 3;
  ByteBufferPutString(buffer, &shift, username, 64);
  ByteBufferPutString(buffer, &shift, password, 64);
  ByteBufferPutString(buffer, &shift, roomName, 64);
  ByteBufferPutInt(buffer, &shift, numberOfUsers);
  return buffer;
}
void *createPacketToSendMessageByClient(char* username, char* roomName, char* message) {
  char *buffer = ByteBufferAllocate(512);
  setOperationOnPacket(buffer, OP_SEND_MESSAGE);
  setTypeOnPacket(buffer, TYPE_CLIENT);
  int shift = sizeof(int) * 3;
  ByteBufferPutString(buffer, &shift, username, 64);
  ByteBufferPutString(buffer, &shift, roomName, 64);
  ByteBufferPutString(buffer, &shift, message, 256);
  return buffer;
}
void *createPacketToExitByClient(char *username, char *password) {
  char *buffer = ByteBufferAllocate(256);
  setOperationOnPacket(buffer, OP_SEND_MESSAGE);
  setTypeOnPacket(buffer, TYPE_CLIENT);
  int shift = sizeof(int) * 3;
  ByteBufferPutString(buffer, &shift, username, 64);
  ByteBufferPutString(buffer, &shift, password, 64);
  return buffer;
}

// Read
void readPacketToListRoomsByClient(char* packet, void (*callback)(int, int, int)) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  if (callback) {
    callback(error, type, op);
  } else {
    printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
    printf("\n");
  }
}
void readPacketToConnectByClient(char* packet, void (*callback)(int, int, int, char*, char*, char*)) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  char* username = ByteBufferGetString(packet, &shift);
  char* password = ByteBufferGetString(packet, &shift);
  char* roomName = ByteBufferGetString(packet, &shift);
  if (callback) {
    callback(error, type, op, username, password, roomName);
  } else {
    printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
    printf(", Username: '%s', Password: '%s', Room name: '%s'\n", username, password, roomName);
  }
}
void readPacketToCreateRoomByClient(char* packet, void (*callback)(int, int, int, char*, char*, char*, int)) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  char* username = ByteBufferGetString(packet, &shift);
  char* password = ByteBufferGetString(packet, &shift);
  char* roomName = ByteBufferGetString(packet, &shift);
  int numberOfUsers = ByteBufferGetInt(packet, &shift);
  if (callback) {
    callback(error, type, op, username, password, roomName, numberOfUsers);
  } else {
    printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
    printf(", Username: '%s', Password: '%s', Room name: '%s', Number of users: '%d'\n", username, password, roomName, numberOfUsers);
  }
}
void readPacketToSendMessageByClient(char* packet, void (*callback)(int, int, int, char*, char*, char*)) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  char* username = ByteBufferGetString(packet, &shift);
  char* roomName = ByteBufferGetString(packet, &shift);
  char* message = ByteBufferGetString(packet, &shift);
  if (callback) {
    callback(error, type, op, username, roomName, message);
  } else {
    printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
    printf(", Username: '%s', RoomName: '%s', Message: '%s'\n", username, roomName, message);
  }
}
void readPacketToExitByClient(char* packet, void (*callback)(int, int, int, char*, char*)) {
  int shift = 0;
  int error = ByteBufferGetInt(packet, &shift);
  int type = ByteBufferGetInt(packet, &shift);
  int op = ByteBufferGetInt(packet, &shift);
  char* username = ByteBufferGetString(packet, &shift);
  char* password = ByteBufferGetString(packet, &shift);
  if (callback) {
    callback(error, type, op, username, password);
  } else {
    printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
    printf(", Username: '%s', Password: '%s'\n", username, password);
  }
}

void testOnePacket(char* packet) {
  if (!getErrorOfPacket(packet)) {
    int op = getOperationOfPacket(packet);
    switch(op) {
      case OP_LIST:
      readPacketToListRoomsByClient(packet, NULL);
      break;
      case OP_CONNECT:
      readPacketToConnectByClient(packet, NULL);
      break;
      case OP_CREATE_ROOM:
      readPacketToCreateRoomByClient(packet, NULL);
      break;
      case OP_SEND_MESSAGE:
      readPacketToSendMessageByClient(packet, NULL);
      break;
      case OP_EXIT:
      readPacketToExitByClient(packet, NULL);
      break;
    }
  }
  free(packet);
}
void testPackets() {
  testOnePacket(createPacketToListRoomsByClient());
  testOnePacket(createPacketToConnectByClient("user-123", "123456", "-room-"));
  testOnePacket(createPacketToCreateRoomByClient("user-123", "123456", "-room-", 2));
  testOnePacket(createPacketToSendMessageByClient("user-123", "-room-", "Hello world!"));
  testOnePacket(createPacketToExitByClient("user-123", "123456"));
}

void testCallback1() {
  void callback(int error, int type, int op, char *username, char *roomName, char *message) {
    printf("Message '%s'\n", message);
  }
  char* packet = createPacketToSendMessageByClient("user-123", "-room-", "Hello world!");
  readPacketToSendMessageByClient(packet, &callback);
  free(packet);
}
void testCallback2() {
  void callback(int error, int type, int op, char *username, char *password, char *roomName, int numbberOfUsers) {
    printf("Username '%s'\n", username);
  }
  char* packet = createPacketToCreateRoomByClient("user-123", "123456", "-room-", 2);
  readPacketToCreateRoomByClient(packet, &callback);
  free(packet);
}

int main(int argc, char const *argv[]) {
  testPackets();
  testCallback1();
  testCallback2();
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
