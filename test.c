#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <nfc/nfc.h>

nfc_target nt;
nfc_context *context;
nfc_device *pnd;

#define UID_SIZE 7
#define BIN_SIZE 540
#define PASSWORD_SIZE 4

#define UID_OFFSET 468
#define PASSWORD_OFFSET 532

uint8_t uid[UID_SIZE];
uint8_t decryptedBin[BIN_SIZE];
uint8_t encryptedBin[BIN_SIZE];
uint8_t bcc[2];
uint8_t password[PASSWORD_SIZE] = {0, 0, 0, 0};


int writePipe[2] = {-1, -1};
int readPipe[2] = {-1, -1};

#define COMMAND_SIZE 1000

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

void replaceUIDInBin() {
  bcc[0] = 0x88 ^ uid[0] ^ uid[1] ^ uid[2];
  bcc[1] = uid[3] ^ uid[4] ^ uid[5] ^ uid[6];

  int i;
  for (i = 0; i < 3; i++) {
    decryptedBin[UID_OFFSET + i] = uid[i];
  }

  decryptedBin[UID_OFFSET + i++] = bcc[0];

  for (; i < 8; i++) {
    decryptedBin[UID_OFFSET + i] = uid[i - 1];
  }
}

void replacePassword() {
  password[0] = 0xAA ^ uid[1] ^ uid[3];
  password[1] = 0x55 ^ uid[2] ^ uid[4];
  password[2] = 0xAA ^ uid[3] ^ uid[5];
  password[3] = 0x55 ^ uid[4] ^ uid[6];

  for (int i = 0; i < PASSWORD_SIZE; i++) {
    decryptedBin[PASSWORD_OFFSET + i] = password[i];
  }
}

void setDefaults() {
  decryptedBin[0] = bcc[1];

  // All of these are magic values
  decryptedBin[536] = 0x80;
  decryptedBin[537] = 0x80;

  decryptedBin[520] = 0;
  decryptedBin[521] = 0;
  decryptedBin[522] = 0;

  decryptedBin[2] = 0;
  decryptedBin[3] = 0;
}

void updateForUID() {
  // Credit: https://gist.githubusercontent.com/ShoGinn/d27a726296f4370bbff0f9b1a7847b85/raw/aeb425e8b1708e1c61f78c3e861dad03c20ca8ab/Arduino_amiibo_tool.bash
  replaceUIDInBin();
  replacePassword();
  setDefaults();
}

void encryptBin(const char* keyPath) {
  if (pipe(writePipe) < 0) {
    fprintf(stderr, "Could not open write pipe\n");
    exit(1);
  }

  if (pipe(readPipe) < 0) {
    fprintf(stderr, "Could not open read pipe\n");
    exit(1);
  }

  int savedStdin = dup(0);
  if (dup2(writePipe[0], 0) < 0) {
    fprintf(stderr, "Could not redirect stdin\n");
    exit(1);
  }

  int savedStdout = dup(1);
  if (dup2(readPipe[1], 1) < 0) {
    fprintf(stderr, "Could not redirect stdout\n");
    exit(1);
  }

  int pipeSize;
  if (BIN_SIZE != (pipeSize = write(writePipe[1], decryptedBin, BIN_SIZE))) {
    fprintf(stderr, "Wrote incorrect size to pipe: %d\n", pipeSize);
    perror("write");
    exit(1);
  }

  const char *staticCommand = "./amiitool/amiitool -e -k ";
  char command[COMMAND_SIZE + strlen(staticCommand)];

  if (strlen(keyPath) >= COMMAND_SIZE) {
    fprintf(stderr, "Key path too big\n");
    exit(1);
  }

  sprintf(command, "%s%s", staticCommand, keyPath);
  system(command);

  if (BIN_SIZE != (pipeSize = read(readPipe[0], encryptedBin, BIN_SIZE))) {
    fprintf(stderr, "Read incorrect size from pipe: %d\n", pipeSize);
    exit(1);
  }
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
  initializeNFC();
  readTag();
  updateForUID();
  encryptBin(argv[1]);
  writeBuffer("test.bin", decryptedBin, BIN_SIZE);
}
