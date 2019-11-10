#include "client.c"

void testOnePacket(char* packet, void* callback) {
  if (!getErrorOfPacket(packet)) {
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
  }
  free(packet);
}
void testPackets() {
  testOnePacket(createPacketToListRoomsByClient(), NULL);
  testOnePacket(createPacketToConnectByClient("user-123", "123456", "-room-"), NULL);
  testOnePacket(createPacketToCreateRoomByClient("user-123", "123456", "-room-", 2), NULL);
  testOnePacket(createPacketToSendMessageByClient("user-123", "-room-", "Hello world!"), NULL);
  testOnePacket(createPacketToExitByClient("user-123", "123456"), NULL);
}

void callback1(int error, int type, int op) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf("\n");
}
void callback2(int error, int type, int op, char *username, char *password, char* roomName) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', Password: '%s', Room name: '%s'\n", username, password, roomName);
}
void callback3(int error, int type, int op, char *username, char *password, char *roomName, int numberOfUsers) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', Password: '%s', Room name: '%s', Number of users: '%d'\n", username, password, roomName, numberOfUsers);
}
void callback4(int error, int type, int op, char *username, char *roomName, char *message) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', RoomName: '%s', Message: '%s'\n", username, roomName, message);
}
void callback5(int error, int type, int op, char *username, char *password) {
  printf("Error: '%d', Type: '%d', Operation: '%d'", error, type, op);
  printf(", Username: '%s', Password: '%s'\n", username, password);
}
void testCallbacks() {
  testOnePacket(createPacketToListRoomsByClient(), &callback1);
  testOnePacket(createPacketToConnectByClient("user-123", "123456", "-room-"), &callback2);
  testOnePacket(createPacketToCreateRoomByClient("user-123", "123456", "-room-", 2), &callback3);
  testOnePacket(createPacketToSendMessageByClient("user-123", "-room-", "Hello world!"), &callback4);
  testOnePacket(createPacketToExitByClient("user-123", "123456"), &callback5);
}

int main(int argc, char const *argv[]) {
  testPackets();
  printf("\n");
  testCallbacks();
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