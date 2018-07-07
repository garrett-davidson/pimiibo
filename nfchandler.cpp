#include "nfchandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <nfc/nfc.h>

#define UID_SIZE 7

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
    print_hex(target->nti.nai.abtUid, uidSize);

    if (UID_SIZE != uidSize) {
      fprintf(stderr, "Read wrong size UID\n");
      exit(1);
    }

    for (int i = 0; i < UID_SIZE; i++) {
      uuidBuffer[i] = target->.nti.nai.abtUid[i];
    }
  }

}
