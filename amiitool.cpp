#include "amiitool.h"
#include "amiibo.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define COMMAND_SIZE 1000

Amiitool *Amiitool::shared() {
  if (!_shared) { _shared = new Amiitool(); }

  return _shared;
}

void Amiitool::setKeyPath(const char *keyPath) {
  Amiitool::_keyPath = keyPath;
}

Amiitool::Amiitool() {
  if (pipe(writePipe) < 0) {
    fprintf(stderr, "Could not open write pipe\n");
    exit(1);
  }

  if (pipe(readPipe) < 0) {
    fprintf(stderr, "Could not open read pipe\n");
    exit(1);
  }

  savedStdin = dup(0);
  savedStdout = dup(1);
}

int Amiitool::decryptBuffer(uint8_t *encryptedBuffer, uint8_t *decryptedBuffer) {
  printf("\nDecrypting bin\n");
  return pipeToAmiitool("-d", Amiitool::_keyPath, encryptedBuffer, decryptedBuffer);
}

int Amiitool::encryptBuffer(uint8_t *encryptedBuffer, uint8_t *decryptedBuffer) {
  printf("Encrypting\n");
  return pipeToAmiitool("-e", Amiitool::_keyPath, decryptedBuffer, encryptedBuffer);
}

int Amiitool::pipeToAmiitool(const char *args, const char* keyPath, uint8_t *inputBuffer, uint8_t *outputBuffer) {
  printf("Sending bin to amiitool...");

  redirectIO();

  int pipeSize;
  if (AMIIBO_SIZE != (pipeSize = write(writePipe[1], inputBuffer, AMIIBO_SIZE))) {
    fprintf(stderr, "Wrote incorrect size to pipe: %d\n", pipeSize);
    perror("write");
    exit(1);
  }

  const char *staticCommand = "./amiitool/amiitool %s -k %s";
  char command[COMMAND_SIZE + strlen(staticCommand)];

  if (strlen(keyPath) >= COMMAND_SIZE) {
    fprintf(stderr, "Key path too big\n");
    exit(1);
  }

  sprintf(command, staticCommand, args, keyPath);
  system(command);

  if (AMIIBO_SIZE != (pipeSize = read(readPipe[0], outputBuffer, AMIIBO_SIZE))) {
    fprintf(stderr, "Read incorrect size from pipe: %d\n", pipeSize);
    exit(1);
  }

  resetIO();

  printf("Done\n");
  return 0;
}

void Amiitool::redirectIO() {
  if (dup2(writePipe[0], 0) < 0) {
    fprintf(stderr, "Could not redirect stdin\n");
    exit(1);
  }

  if (dup2(readPipe[1], 1) < 0) {
    fprintf(stderr, "Could not redirect stdout\n");
    exit(1);
  }
}

void Amiitool::resetIO() {
  if (dup2(savedStdin, 0) < 0) {
    fprintf(stderr, "Could not reset stdin\n");
    exit(1);
  }

  if (dup2(savedStdout, 1) < 0) {
    fprintf(stderr, "Could not reset stdout\n");
    exit(1);
  }
}
