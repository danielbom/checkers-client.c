#include <stdlib.h>
#include <netinet/in.h>
#include "byte-buffer.c"

// Alias to increase readability
typedef struct sockaddr_in SocketAddrIn;
typedef struct sockaddr SocketAddr;

// Alias to increase readability
typedef struct sockaddr_in SocketAddrIn;
typedef struct sockaddr SocketAddr;

enum {
  OP_LIST = 1,
  OP_CONNECT = 2,
  OP_CREATE_ROOM = 3,
  OP_SEND_MESSAGE = 4,
  OP_EXIT = 5
};

enum {
  TYPE_CLIENT = 1,
  TYPE_SERVER = 2
};

int PROTOCOL_ID = 28456;

// Gets
int getErrorOfPacket(char* packet) {
  return ByteBufferGetIntAbs(packet, 0);
}
int getTypeOfPacket(char* packet) {
  return ByteBufferGetIntAbs(packet, sizeof(int));
}
int getOperationOfPacket(char* packet) {
  return ByteBufferGetIntAbs(packet, sizeof(int) * 2);
}
int getIDProtoocolPacket(char* packet) {
  return ByteBufferGetIntAbs(packet, sizeof(int) * 3);
}

// Sets
void setErrorOnPacket(char* packet, int value) {
  ByteBufferPutIntAbs(packet, 0, value);
}
void setTypeOnPacket(char* packet, int value) {
  ByteBufferPutIntAbs(packet, sizeof(int), value);
}
void setOperationOnPacket(char* packet, int value) {
  ByteBufferPutIntAbs(packet, sizeof(int) * 2, value);
}
void setIDProtocolPacket(char* packet) {
  ByteBufferPutIntAbs(packet, sizeof(int) * 3, PROTOCOL_ID);
}

// Utils
void rejectCriticalError(char* message, int isCriticalError) {
  if (isCriticalError) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}

void showHostInfos(int sd) {
  int addrLength;
  struct sockaddr_in addr;
  getpeername(sd, (struct sockaddr*)&addr, &addrLength);
  printf("%d = %s:%d\n", sd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

int isNull(char* str) {
  return str == NULL;
}

void checkErrorOfServer(int error) {
  if (error) printf("ERROR: ");
  switch (error) {
    case 1:
      printf("Username is null\n");
      break;
    case 2:
      printf("Password is null\n");
      break;
    case 3:
      printf("Room name is null\n");
      break;
    case 4:
      printf("Message is null\n");
      break;
  }
}
