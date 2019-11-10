#include <string.h>

void *ByteBufferAllocate(int size) {
  return calloc(size, 1);
}

// Absolute
int ByteBufferPutByteAbs(void *buffer, int shift, char value) {
  int size = sizeof(char);
  memcpy(buffer + shift, &value, size);
  return size;
}
int ByteBufferPutIntAbs(void *buffer, int shift, int value) {
  int size = sizeof(int);
  memcpy(buffer + shift, &value, size);
  return size;
}
int ByteBufferPutStringAbs(void *buffer, int shift, char* value, int n) {
  int length = strlen(value);
  strncat(buffer + shift, value, n - 1);
  n = (length > n ? n : length) + 1;
  char* sbuffer = buffer + shift + n;
  *sbuffer = 0;
  return n;
}

char ByteBufferGetByteAbs(void *buffer, int shift) {
  char value;
  memcpy(&value, buffer + shift, sizeof(char));
  return value;
}
int ByteBufferGetIntAbs(void *buffer, int shift) {
  int value;
  memcpy(&value, buffer + shift, sizeof(int));
  return value;
}
char* ByteBufferGetStringAbs(void *buffer, int shift) {
  return buffer + shift;
}

// Relative
void ByteBufferPutByte(void *buffer, int *shift, char value) {
  *shift += ByteBufferPutByteAbs(buffer, *shift, value);
}
void ByteBufferPutInt(void *buffer, int *shift, int value) {
  *shift += ByteBufferPutIntAbs(buffer, *shift, value);
}
void ByteBufferPutString(void *buffer, int *shift, char *value, int n) {
  *shift += ByteBufferPutStringAbs(buffer, *shift, value, n);
}

char ByteBufferGetByte(void *buffer, int *shift) {
  char value = ByteBufferGetByteAbs(buffer, *shift);
  *shift += sizeof(char);
  return value;
}
int ByteBufferGetInt(void *buffer, int *shift) {
  int value = ByteBufferGetIntAbs(buffer, *shift);
  *shift += sizeof(int);
  return value;
}
char *ByteBufferGetString(void *buffer, int *shift) {
  char* value = ByteBufferGetStringAbs(buffer, *shift);
  *shift += strlen(value) + 1;
  return value;
}
