#include <stdio.h>
#include <stdlib.h>
#include "byte-buffer.c"

int main() {
  char *buffer = ByteBufferAllocate(256);
  int shift = 0;
  ByteBufferPutByte(buffer, &shift, 100);
  ByteBufferPutByte(buffer, &shift, 20);
  ByteBufferPutByte(buffer, &shift, 10);

  ByteBufferPutInt(buffer, &shift, 1000);
  ByteBufferPutInt(buffer, &shift, 2000);
  ByteBufferPutInt(buffer, &shift, 3000);

  ByteBufferPutString(buffer, &shift, "Hello world", 5);
  ByteBufferPutString(buffer, &shift, "Hello world", 5);
  ByteBufferPutString(buffer, &shift, "Hello world", 5);

  int shift2 = 0;
  printf("%d\n", ByteBufferGetByte(buffer, &shift2));
  printf("%d\n", ByteBufferGetByte(buffer, &shift2));
  printf("%d\n", ByteBufferGetByte(buffer, &shift2));

  printf("%d\n", ByteBufferGetInt(buffer, &shift2));
  printf("%d\n", ByteBufferGetInt(buffer, &shift2));
  printf("%d\n", ByteBufferGetInt(buffer, &shift2));

  printf("%s\n", ByteBufferGetString(buffer, &shift2));
  printf("%s\n", ByteBufferGetString(buffer, &shift2));
  printf("%s\n", ByteBufferGetString(buffer, &shift2));

  free(buffer);
}