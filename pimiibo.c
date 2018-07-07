#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <nfc/nfc.h>

#define PAGE_COUNT 135

#define WRITE_COMMAND 0xa2

const uint8_t dynamicLockBytes[4] = { 0x01, 0x00, 0x0f, 0xbd };
const uint8_t staticLockBytes[4] = { 0x00, 0x00, 0x0F, 0xE0 };

static void print_hex(const uint8_t *pbtData, const size_t szBytes)
{
  size_t  szPos;

  for (szPos = 0; szPos < szBytes; szPos++) {
    printf("%02x  ", pbtData[szPos]);
  }
  printf("\n");
}

void writeBuffer(const char* path, uint8_t *buffer, size_t size) {
  FILE *file = fopen(path, "w");
  if (!file) {
    fprintf(stderr, "Could not open %s\n", path);
    exit(1);
  }

  if (size != fwrite(buffer, 1, size, file)) {
    fprintf(stderr, "Could not write to file\n");
    exit(1);
  }
  fclose(file);
}

void setDefaults() {
}

void encryptBin(const char* keyPath) {
  printf("Encrypting\n");
  pipeToAmiitool("-e", keyPath, decryptedBin, encryptedBin);
  printf("Encrypted\n\n");
}

void writePage(uint8_t page, const uint8_t *pageData) {
  printf("Writing to %d: %02x %02x %02x %02x...",
         page, pageData[0], pageData[1], pageData[2], pageData[3]);

  uint8_t sendData[6] = {
    WRITE_COMMAND, page, pageData[0], pageData[1], pageData[2], pageData[3]
  };

  int responseCode = nfc_initiator_transceive_bytes(pnd, sendData, 6, NULL, 0, 0);

  if (responseCode == 0) {
    printf("Done\n");
  } else {
    printf("Failed\n");
    fprintf(stderr, "Failed to write to tag\n");
    nfc_perror(pnd, "Write");
    exit(1);
  }
}

void writeData() {
  printf("Writing encrypted bin:\n");
  for (uint8_t i = 3; i < PAGE_COUNT; i++) {
    writePage(i, encryptedBin + (i * 4));
  }
  printf("Done\n");
}

void writeDynamicLockBytes() {
  printf("Writing dynamic lock bytes\n");
  writePage(130, dynamicLockBytes);
  printf("Done\n");
}

void writeStaticLockBytes() {
  printf("Writing static lock bytes\n");
  writePage(2, staticLockBytes);
  printf("Done\n");
}

void writeTag() {
  printf("Writing tag:\n");
  writeData();
  writeDynamicLockBytes();
  writeStaticLockBytes();
  printf("Finished writing tag\n");
}

void printUsage() {
  printf("pimiibo keyfile binfile\n");
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "Incorrect number of arguments\n");
    printUsage();
    exit(1);
  }

  readEncryptedBin(argv[2]);
  decryptBin(argv[1]);
  initializeNFC();
  readTag();
  updateForUID();
  encryptBin(argv[1]);
  writeTag();
}
