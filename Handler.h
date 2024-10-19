#ifndef PICO_MODBUS_HANDLER_H
#define PICO_MODBUS_HANDLER_H

#include <cstdio>
#include <memory>
#include "Controller.h"

class Handler {
public:
    Handler(std::weak_ptr<Controller> Controller) : controller(Controller) {};
    uint8_t read(uint16_t memory_address);
    void write(uint16_t memory_address, uint8_t data);
    void load();
    void save();
private:
    std::weak_ptr<Controller> controller;
};


#endif //PICO_MODBUS_HANDLER_H