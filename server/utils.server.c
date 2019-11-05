#include <stdlib.h>
#include <netinet/in.h>

// Alias to increase readability
typedef struct sockaddr_in SocketAddrIn;
typedef struct sockaddr SocketAddr;

// Alias to increase readability
typedef struct sockaddr_in SocketAddrIn;
typedef struct sockaddr SocketAddr;

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
