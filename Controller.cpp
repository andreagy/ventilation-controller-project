#include "Controller.h"

void Controller::setSpeed(int speed) {
    if ((this->current_speed + speed > 1000) || (this->current_speed - speed > 1000)) this->current_speed = 1000;
    else if ((this->current_speed + speed < 0) || (this->current_speed - speed < 0)) this->current_speed = 0;
    else this->current_speed = speed;
}

void Controller::setAuto(int mode) {
    if (mode == 0 || mode == 1) this->auto_toggle = mode;
}

void Controller::setPressure(uint8_t pressure_c) {
    if ((pressure_c >= 0) && (pressure_c <= 125)) this->pressure = pressure_c;
}

int Controller::getSpeed() {
    return this->current_speed;
}

int Controller::getAuto() {
    return this->auto_toggle;
}

uint8_t Controller::getPressure() {
    return this->pressure;
}