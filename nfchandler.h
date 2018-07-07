#include <stdlib.h>

struct nfc_context;
struct nfc_device;
struct nfc_target;

class NFCHandler {
public:
  NFCHandler();
  void readTagUUID(uint8_t uuidBuffer[]);

private:
  struct nfc_target *target;
  nfc_context *context;
  nfc_device *device;
};
