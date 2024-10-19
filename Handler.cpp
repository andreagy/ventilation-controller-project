#include <pico/time.h>
#include <hardware/i2c.h>
#include "Handler.h"
#define DEVICE_ADDR 0x50
#define MEMORY_I2C i2c0
#define auto_addr_inverted 0x7ff6
#define auto_addr 0x7fec
#define pressure_addr 0x7fe2
#define speed_addr 0x7fd8

uint8_t Handler::read(uint16_t memory_address) {
    uint8_t value;
    uint8_t buffer[2];
    buffer[0] = memory_address >> 8;
    buffer[1] = memory_address & 0xFF;
    i2c_write_blocking(MEMORY_I2C, DEVICE_ADDR, buffer, 2, false);
    sleep_ms(5);
    i2c_read_blocking(MEMORY_I2C, DEVICE_ADDR, &value, 1, false);
    return value;
}

void Handler::write(uint16_t memory_address, uint8_t data) {
    uint8_t buffer[3];
    buffer[0] = memory_address >> 8;
    buffer[1] = memory_address & 0xFF;
    buffer[2] = data;
    i2c_write_blocking(MEMORY_I2C, DEVICE_ADDR, buffer, 3, false);
    sleep_ms(5);
}

void Handler::load() {
    std::shared_ptr<Controller> conny = this->controller.lock();
    uint8_t isAuto = read(auto_addr);
    uint8_t isNotAuto = read(auto_addr_inverted);
    (isAuto != isNotAuto) ? conny->setAuto(1) : conny->setAuto(0);
    conny->setPressure(read(pressure_addr));
    conny->setSpeed((read(speed_addr) * 10));
}

void Handler::save() {
    std::shared_ptr<Controller> conny = this->controller.lock();
    write(auto_addr, conny->getAuto());
    write(auto_addr_inverted, !conny->getAuto());
    write(speed_addr, (int)(conny->getSpeed()/10));
    write(pressure_addr, conny->getPressure());
}
