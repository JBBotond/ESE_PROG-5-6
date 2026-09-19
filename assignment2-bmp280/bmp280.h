#include <avr/io.h>

class Bmp280 {
  public:
    uint8_t getId();
    void reset();

  private:
    uint8_t id;
};