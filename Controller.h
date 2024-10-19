#ifndef PICO_MODBUS_CONTROLLER_H
#define PICO_MODBUS_CONTROLLER_H

#include <cstdio>
#include <cstdint>

class Controller {
public:
    void setSpeed(int speed);
    void setAuto(int mode);
    void setPressure(uint8_t pressure);
    int getSpeed();
    int getAuto();
    uint8_t getPressure();
private:
    int current_speed = 0;
    int auto_toggle = 0;
    uint8_t pressure = 0;
};


#endif //PICO_MODBUS_CONTROLLER_H