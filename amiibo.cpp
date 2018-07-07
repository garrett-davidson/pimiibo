#include "amiibo.h"
#include "amiitool.h"

#include <stdio.h>

#define UUID_OFFSET 468
#define PASSWORD_OFFSET 532
#define PASSWORD_SIZE 4

Amiibo::Amiibo(const char* filePath) {
  readFileIntoBuffer(filePath, encryptedBuffer, AMIIBO_SIZE);

  Amiitool::shared()->decryptBuffer(encryptedBuffer, buffer);
}

void Amiibo::setUUID(const uint8_t *uuid) {
  printf("\nUpdating bin for new UID:\n");

  // Credit: https://gist.githubusercontent.com/ShoGinn/d27a726296f4370bbff0f9b1a7847b85/raw/aeb425e8b1708e1c61f78c3e861dad03c20ca8ab/Arduino_amiibo_tool.bash
  replaceWithUUID(uuid);
  replacePassword(uuid);
  setDefaults(uuid);
  Amiitool::shared()->encryptBuffer(encryptedBuffer, buffer);

  printf("Finished updating bin\n\n");
}

void Amiibo::readFileIntoBuffer(const char *filePath, uint8_t *buffer, size_t size) {
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

void Amiibo::replaceWithUUID(const uint8_t uuid[]) {
  uint8_t bcc[2];

  printf("Replacing UID\n");
  bcc[0] = 0x88 ^ uuid[0] ^ uuid[1] ^ uuid[2];
  bcc[1] = uuid[3] ^ uuid[4] ^ uuid[5] ^ uuid[6];

  int i;
  for (i = 0; i < 3; i++) {
    buffer[UUID_OFFSET + i] = uuid[i];
  }

  buffer[UUID_OFFSET + i++] = bcc[0];

  for (; i < 8; i++) {
    buffer[UUID_OFFSET + i] = uuid[i - 1];
  }
}

void Amiibo::replacePassword(const uint8_t uuid[]) {
  uint8_t password[PASSWORD_SIZE] = {0, 0, 0, 0};

  printf("Updating password\n");
  password[0] = 0xAA ^ uuid[1] ^ uuid[3];
  password[1] = 0x55 ^ uuid[2] ^ uuid[4];
  password[2] = 0xAA ^ uuid[3] ^ uuid[5];
  password[3] = 0x55 ^ uuid[4] ^ uuid[6];

  for (int i = 0; i < PASSWORD_SIZE; i++) {
    buffer[PASSWORD_OFFSET + i] = password[i];
  }
}

void Amiibo::setDefaults(const uint8_t uuid[]) {
  printf("Writing magic bytes\n");

  // Same as bcc[1]
  buffer[0] = uuid[3] ^ uuid[4] ^ uuid[5] ^ uuid[6];

  // All of these are magic values
  buffer[536] = 0x80;
  buffer[537] = 0x80;

  buffer[520] = 0;
  buffer[521] = 0;
  buffer[522] = 0;

  buffer[2] = 0;
  buffer[3] = 0;
}
