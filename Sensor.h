#ifndef PICO_MODBUS_SENSOR_H
#define PICO_MODBUS_SENSOR_H
#include <cstdio>
#include <cstdint>

class Sensor {
public:
    uint16_t measure();
};


#endif //PICO_MODBUS_SENSOR_H