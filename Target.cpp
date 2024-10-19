#include "Target.h"

void Target::setSSID(std::string SSID) {
    this->ssid = std::move(SSID);
}

void Target::setPassword(std::string Password) {
    this->password = std::move(Password);
}

void Target::setHostname(std::string Hostname) {
    this->hostname = std::move(Hostname);
}

void Target::setPort(int Port) {
    this->port = Port;
}

std::string Target::getSSID() {
    return this->ssid;
}

std::string Target::getPassword() {
    return this->password;
}

std::string Target::getHostname() {
    return this->hostname;
}

int Target::getPort() const {
    return this->port;
}