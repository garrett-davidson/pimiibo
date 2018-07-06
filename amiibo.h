#define AMIIBO_SIZE 540

#include <stdlib.h>

class Amiibo {
public:
  Amiibo(const char *filePath);

  void encryptIntoBuffer(char *buffer);

private:
  uint8_t buffer[AMIIBO_SIZE];

  void readFileIntoBuffer(const char *filePath, uint8_t *buffer, size_t size);
};
