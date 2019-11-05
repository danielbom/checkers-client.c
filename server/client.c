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
      break;
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

int main(int argc, char const *argv[]) {
  int valread;

  initClientSocket();

  pthread_t thread;
  senderRunner(&thread);
  receiverRunner(&thread, NULL);

  printf("Loop...\n");
  while (runningClient) {}
  return 0;
}
