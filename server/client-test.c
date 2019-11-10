#include "client.c"

void testOnePacket(char* packet, void* callback) {
  int error = getErrorOfPacket(packet);
  if (!error) {
    int op = getOperationOfPacket(packet);
    switch(op) {
      case OP_LIST:
      readPacketToListRoomsByClient(packet, callback);
      break;
      case OP_CONNECT:
      readPacketToConnectByClient(packet, callback);
      break;
      case OP_CREATE_ROOM:
      readPacketToCreateRoomByClient(packet, callback);
      break;
      case OP_SEND_MESSAGE:
      readPacketToSendMessageByClient(packet, callback);
      break;
      case OP_EXIT:
      readPacketToExitByClient(packet, callback);
      break;
    }
  } else {
    checkErrorOfServer(error);
  }
  free(packet);
}
void testPackets() {
  testOnePacket(createPacketToListRoomsByClient(), NULL);
  testOnePacket(createPacketToConnectByClient("user-x", "123456", "-room-"), NULL);
  testOnePacket(createPacketToCreateRoomByClient("user-x", "123456", "-room-", 2), NULL);
  testOnePacket(createPacketToSendMessageByClient("user-x", "-room-", "Hello world!"), NULL);
  testOnePacket(createPacketToExitByClient("user-x", "123456"), NULL);
}

void callbackListRooms(int error, int type, int op) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf("\n");
}
void callbackConnect(int error, int type, int op, char *username, char *password, char* roomName) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', Password: '%s', Room name: '%s'\n", username, password, roomName);
}
void callbackCreateRoom(int error, int type, int op, char *username, char *password, char *roomName, int numberOfUsers) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', Password: '%s', Room name: '%s', Number of users: '%d'\n", username, password, roomName, numberOfUsers);
}
void callbackSendMessage(int error, int type, int op, char *username, char *roomName, char *message) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', RoomName: '%s', Message: '%s'\n", username, roomName, message);
}
void callbackExit(int error, int type, int op, char *username, char *password) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', Password: '%s'\n", username, password);
}
void testCallbacks() {
  testOnePacket(createPacketToListRoomsByClient(), &callbackListRooms);
  testOnePacket(createPacketToConnectByClient("user-123", "123456", "-room-"), &callbackConnect);
  testOnePacket(createPacketToCreateRoomByClient("user-123", "123456", "-room-", 2), &callbackCreateRoom);
  testOnePacket(createPacketToSendMessageByClient("user-123", "-room-", "Hello world!"), &callbackSendMessage);
  testOnePacket(createPacketToExitByClient("user-123", "123456"), &callbackExit);
}

void testErrors() {
  testOnePacket(createPacketToConnectByClient(NULL, "123456", "-room-"), NULL);
  testOnePacket(createPacketToCreateRoomByClient("user-123", "123456", NULL, 2), NULL);
  testOnePacket(createPacketToSendMessageByClient("user-123", "-room-", NULL), NULL);
  testOnePacket(createPacketToExitByClient("user-123", NULL), NULL);
  printf("\n");
  testOnePacket(createPacketToConnectByClient(NULL, NULL, NULL), NULL);
  testOnePacket(createPacketToCreateRoomByClient(NULL, NULL, NULL, 2), NULL);
  testOnePacket(createPacketToSendMessageByClient(NULL, NULL, NULL), NULL);
  testOnePacket(createPacketToExitByClient(NULL, NULL), NULL);
}

void testsClient() {
  testPackets();
  printf("\n");
  testCallbacks();
  printf("\n");
  testErrors();
}

void connectClient() {
  int valread;

  initClientSocket();

  pthread_t thread;
  senderRunner(&thread);
  receiverRunner(&thread, NULL);

  printf("Loop...\n");
  while (runningClient) {}
}

int main(int argc, char const *argv[]) {
  connectClient();
  return 0;
}