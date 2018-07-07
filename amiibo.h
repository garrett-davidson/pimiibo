#define AMIIBO_SIZE 540

#include <stdlib.h>

class Amiibo {
public:
  Amiibo(const char *filePath);

  void updateForUUID(const uint8_t uuid[]);

private:
  uint8_t buffer[AMIIBO_SIZE];

  void readFileIntoBuffer(const char *filePath, uint8_t *buffer, size_t size);

  void replaceWithUUID(const uint8_t uuid[]);
  void replacePassword(const uint8_t uuid[]);
  void setDefaults(const uint8_t uuid[]);
};
