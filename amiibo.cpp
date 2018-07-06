#include "amiibo.h"
#include <stdio.h>

Amiibo::Amiibo(const char* filePath) {
  uint8_t encryptedBuffer[AMIIBO_SIZE];
  readFileIntoBuffer(filePath, encryptedBuffer, AMIIBO_SIZE);
}

void readFileIntoBuffer(const char *filePath, uint8_t *buffer, size_t size) {
  FILE *file = fopen(filePath, "r");

  if (!file) {
    fprintf(stderr, "Could not open %s\n", filePath);
    exit(1);
  }

  if (size != fread(buffer, 1, size, file)) {
    fprintf(stderr, "Read incorrect number of bytes from file: %s\n", filePath);
    exit(1);
  }

  fclose(file);
}
