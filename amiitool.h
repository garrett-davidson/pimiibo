#include <cstdlib>
#include <cstdint>

class Amiitool {
public:
  Amiitool();
  int decryptBuffer(uint8_t encryptedBuffer[], uint8_t *decryptedBuffer);
  int encryptBuffer(uint8_t encryptedBuffer[], uint8_t *decryptedBuffer);

  static Amiitool *shared();
  static void setKeyPath(const char *keyPath);

  void printHex(const uint8_t *buffer, const size_t size);

private:
  int writePipe[2];
  int readPipe[2];
  int savedStdin;
  int savedStdout;

  static Amiitool *_shared;
  static const char *_keyPath;

  int pipeToAmiitool(const char *args, const char* keyPath, uint8_t *inputBuffer, uint8_t *outputBuffer);
  void redirectIO();
  void resetIO();
};
