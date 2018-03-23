#include <stdlib.h>
#include <nfc/nfc.h>

nfc_target nt;
nfc_context *context;
nfc_device *pnd;

#define UID_SIZE 7
#define BIN_SIZE 540
#define KEY_SIZE 160

uint8_t uid[UID_SIZE];
uint8_t decryptedBin[BIN_SIZE];
uint8_t encryptedBin[BIN_SIZE];
uint8_t key[KEY_SIZE];

static void print_hex(const uint8_t *pbtData, const size_t szBytes)
{
  size_t  szPos;

  for (szPos = 0; szPos < szBytes; szPos++) {
    printf("%02x  ", pbtData[szPos]);
  }
  printf("\n");
}

void initializeNFC() {
  printf("Initializing NFC adapter\n");
  nfc_init(&context);

  if (!context) {
    printf("Unable to init libnfc (malloc)\n");
    exit(EXIT_FAILURE);
  }

  pnd = nfc_open(context, NULL);

  if (pnd == NULL) {
    printf("ERROR: %s\n", "Unable to open NFC device.");
    exit(EXIT_FAILURE);
  }
  if (nfc_initiator_init(pnd) < 0) {
    nfc_perror(pnd, "nfc_initiator_init");
    exit(EXIT_FAILURE);
  }

  printf("NFC reader: opened\n");
}

void readFileIntoBuffer(const char *path, uint8_t *buffer, size_t size) {
  FILE *file = fopen(path, "r");
  if (!file) {
    fprintf(stderr, "Could not open %s\n", path);
    exit(1);
  }

  if (size != fread(buffer, 1, size, file)) {
    fprintf(stderr, "Read incorrect number of bytes from file: %s\n", path);
    exit(1);
  }

}

void readDecryptedBin(const char *path) {
  printf("Reading bin file\n");
  readFileIntoBuffer(path, decryptedBin, BIN_SIZE);
}

void readKey(const char *path) {
  printf("Reading key file\n");
  readFileIntoBuffer(path, key, KEY_SIZE);
}

void readTag() {
  printf("***Scan tag***\n");
  const nfc_modulation nmMifare = {
    .nmt = NMT_ISO14443A,
    .nbr = NBR_106,
  };

  if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) > 0) {
    printf("Read UID: ");
    int uidSize = nt.nti.nai.szUidLen;
    print_hex(nt.nti.nai.abtUid, uidSize);

    if (UID_SIZE != uidSize) {
      fprintf(stderr, "Read wrong size UID\n");
      exit(1);
    }

    for (int i = 0; i < UID_SIZE; i++) {
      uid[i] = nt.nti.nai.abtUid[i];
    }
  }
}

void encryptBin() {

}

void printUsage() {

}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "Incorrect number of arguments\n");
    printUsage();
    exit(1);
  }

  readDecryptedBin(argv[2]);
  readKey(argv[1]);
  initializeNFC();
  readTag();
  encryptBin();
}
