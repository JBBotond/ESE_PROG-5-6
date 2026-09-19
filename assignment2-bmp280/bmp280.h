#include <avr/io.h>

class Bmp280 {
  public:
    uint8_t getId();
    void reset();
    int getPress();
    int getTemp();
    void config();

  private:
    uint8_t id;
};