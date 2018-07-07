#include "nfchandler.h"
#include "amiitool.h"

#include <stdio.h>
#include <stdlib.h>
#include <nfc/nfc.h>

#define PAGE_COUNT 135
#define WRITE_COMMAND 0XA2

const uint8_t dynamicLockBytes[4] = { 0x01, 0x00, 0x0f, 0xbd };
const uint8_t staticLockBytes[4] = { 0x00, 0x00, 0x0F, 0xE0 };

const nfc_modulation nmMifare = {
  .nmt = NMT_ISO14443A,
  .nbr = NBR_106,
};

NFCHandler::NFCHandler() {
  printf("Initializing NFC adapter\n");
  nfc_init(&context);

  if (!context) {
    printf("Unable to init libnfc (malloc)\n");
    exit(1);
  }

  device = nfc_open(context, NULL);

  if (device == NULL) {
    printf("ERROR: %s\n", "Unable to open NFC device.");
    exit(1);
  }

  if (nfc_initiator_init(device) < 0) {
    nfc_perror(device, "nfc_initiator_init");
    exit(1);
  }

  printf("NFC reader: opened\n");
}

void NFCHandler::readTagUUID(uint8_t uuidBuffer[]) {
  printf("***Scan tag***\n");

  if (nfc_initiator_select_passive_target(device, nmMifare, NULL, 0, target) > 0) {
    printf("Read UID: ");
    int uidSize = target->nti.nai.szUidLen;
    Amiitool::shared()->printHex(target->nti.nai.abtUid, uidSize);

    if (UUID_SIZE != uidSize) {
      fprintf(stderr, "Read wrong size UID\n");
      exit(1);
    }

    for (int i = 0; i < UUID_SIZE; i++) {
      uuidBuffer[i] = target->nti.nai.abtUid[i];
    }
  }
}

void NFCHandler::writeBuffer(const uint8_t *buffer) {
  printf("Writing tag:\n");
  writeDataPages(buffer);
  writeDynamicLockBytes();
  writeStaticLockBytes();
  printf("Finished writing tag\n");
}

void NFCHandler::writeDataPages(const uint8_t *buffer) {
  printf("Writing encrypted bin:\n");
  for (uint8_t i = 3; i < PAGE_COUNT; i++) {
    writePage(i, buffer + (i * 4));
  }
  printf("Done\n");
}

void NFCHandler::writeDynamicLockBytes() {
  printf("Writing dynamic lock bytes\n");
  writePage(130, dynamicLockBytes);
  printf("Done\n");
}

void NFCHandler::writeStaticLockBytes() {
  printf("Writing static lock bytes\n");
  writePage(2, staticLockBytes);
  printf("Done\n");
}

void NFCHandler::writePage(uint8_t page, const uint8_t *buffer) {
    printf("Writing to %d: %02x %02x %02x %02x...",
         page, buffer[0], buffer[1], buffer[2], buffer[3]);

  uint8_t sendData[6] = {
    WRITE_COMMAND, page, buffer[0], buffer[1], buffer[2], buffer[3]
  };

  int responseCode = nfc_initiator_transceive_bytes(device, sendData, 6, NULL, 0, 0);

  if (responseCode == 0) {
    printf("Done\n");
  } else {
    printf("Failed\n");
    fprintf(stderr, "Failed to write to tag\n");
    nfc_perror(device, "Write");
    exit(1);
  }
}
