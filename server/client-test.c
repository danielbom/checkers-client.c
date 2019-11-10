#include "client.c"

void testOnePacket(char* packet) {
  int error = getErrorOfPacket(packet);
  if (!error) {
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
      case OP_SEND_MESSAGE:
      readPacketToSendMessageByClient(packet);
      break;
      case OP_EXIT:
      readPacketToExitByClient(packet);
      break;
    }
  } else {
    checkErrorOfServer(error);
  }
  free(packet);
}
void testPackets() {
  int size;
  testOnePacket(createPacketToListRoomsByClient(&size));
  testOnePacket(createPacketToConnectByClient("user-x", "123456", "-room-", &size));
  testOnePacket(createPacketToCreateRoomByClient("user-x", "123456", "-room-", 2, &size));
  testOnePacket(createPacketToSendMessageByClient("user-x", "-room-", "Hello world!", &size));
  testOnePacket(createPacketToExitByClient("user-x", "123456", &size));
}

void testErrors() {
  int size;
  testOnePacket(createPacketToConnectByClient(NULL, "123456", "-room-", &size));
  testOnePacket(createPacketToCreateRoomByClient("user-123", "123456", NULL, 2, &size));
  testOnePacket(createPacketToSendMessageByClient("user-123", "-room-", NULL, &size));
  testOnePacket(createPacketToExitByClient("user-123", NULL, &size));
  printf("\n");
  testOnePacket(createPacketToConnectByClient(NULL, NULL, NULL, &size));
  testOnePacket(createPacketToCreateRoomByClient(NULL, NULL, NULL, 2, &size));
  testOnePacket(createPacketToSendMessageByClient(NULL, NULL, NULL, &size));
  testOnePacket(createPacketToExitByClient(NULL, NULL, &size));
}

void testsClient() {
  testPackets();
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