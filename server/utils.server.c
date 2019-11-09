#include <stdlib.h>
#include <netinet/in.h>

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

// Gets
int getErrorOfPacket(char* packet) {
  int error;
  memcpy(&error, packet, sizeof(int));
  return error;
}
int getTypeOfPacket(char* packet) {
  int type;
  memcpy(&type, packet + sizeof(int), sizeof(int));
  return type;
}
int getOperationOfPacket(char* packet) {
  int op;
  memcpy(&op, packet + (sizeof(int) * 2), sizeof(int));
  return op;
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
