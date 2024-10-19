#include "Sensor.h"
#include <hardware/i2c.h>
#include "pico/time.h"

#define SCALE_FACTOR 240
#define ALTITUDE_CORR_FACTOR 0.95

uint16_t Sensor::measure() {
    uint8_t pressure_read[2];
    uint8_t start[] = {0xF1};
    i2c_write_blocking(i2c1, 0x40, start, 1, false);
    sleep_ms(10);
    i2c_read_blocking(i2c1, 0x40, pressure_read, 2, false);
    sleep_ms(100);
    uint16_t measurement = ((pressure_read[0] << 8) | pressure_read[1]) / SCALE_FACTOR * ALTITUDE_CORR_FACTOR;
    if (measurement>140){
        measurement = 0; //Hard fixed for unexpected values of pressure greater than 140, for example 250
    }
    return measurement;
}