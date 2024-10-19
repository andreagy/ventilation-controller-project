#ifndef PICO_MODBUS_TARGET_H
#define PICO_MODBUS_TARGET_H


#include <string>
#include <utility>

class Target {
public:
    Target(std::string SSID, std::string Password, std::string Hostname, int Port) : ssid(std::move(SSID)), password(std::move(Password)), hostname(std::move(Hostname)), port(Port) {};
    void setSSID(std::string SSID);
    void setPassword(std::string Password);
    void setHostname(std::string Hostname);
    void setPort(int port);
    std::string getSSID();
    std::string getPassword();
    std::string getHostname();
    int getPort() const;
private:
    std::string ssid;
    std::string password;
    std::string hostname;
    int port;
};


#endif //PICO_MODBUS_TARGET_H