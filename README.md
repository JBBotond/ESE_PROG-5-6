Repository for completing prog 5 and prog 6 assignments

What works:
    - bme280 temperature and humidity reading on Arduino, MCXN947 and rpi platform
    - mqtt broker and publisher running on pi, broadcasting the sensor data

Boards used:
    - Arduino library tested on atmega328p chip on Xplained mini board
    - NXP MCXN947 library tested on NXP MCXN947 board
    - rpi library tested on pi zero board (no wireless)

How to build:
    - Arduino project build under Arduino IDE
    - NXP project builds under MCUxpresso extension under VSCode
    - rpi project builds with CMake

Expected driver outputs:
    sensor ready at 0x76
    T =  22.49 C
    P =  102441.0 Pa
