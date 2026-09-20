#include <avr/io.h>

class Bmp280 {
  public:
    uint8_t getId();
    void reset();
    // Returns the uncompensated 20-bit pressure ADC value.  Calibration data
    // must be applied separately to convert this to pressure in Pa.
    uint32_t getPress();
    uint32_t getTemp();
    float convertPress(uint32_t adc_p);
    int convertTemp(uint32_t adc_t);
    void config();
    void getCalibration();

  private:
    uint8_t id;
    uint16_t dig_T1, dig_T2, dig_T3;
    uint16_t dig_P1, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint32_t t_fine, p_fine;
};
