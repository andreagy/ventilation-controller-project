#include <cstdio>
#include <cstring>
#include <iostream>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/timer.h"
#include "uart/PicoUart.h"

#include "IPStack.h"
#include "Countdown.h"
#include "MQTTClient.h"
#include "ModbusClient.h"
#include "ModbusRegister.h"
#include "ssd1306.h"
#include "Controller.h"
#include "Target.h"
#include "Handler.h"
#include "Sensor.h"

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.

#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define ROT_A_PIN 10
#define ROT_B_PIN 11
#define ROT_SW_PIN 12
#define led_pin 22
#define button 9
std::shared_ptr<Controller> mfic = std::make_shared<Controller>();


#define BAUD_RATE 9600
#define STOP_BITS 1 // for simulator
//#define STOP_BITS 2 // for real system





static const uint8_t ventilator40x40[] =
        {
                0xff, 0x7f, 0x0f, 0x07, 0xc3, 0xe3, 0xf3, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0x71,
                0x71, 0x71, 0x71, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1,
                0xf1, 0xf3, 0xe3, 0xc3, 0x07, 0x0f, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xc7, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0x3f,
                0x1f, 0x0f, 0x0f, 0x07, 0x07, 0x0f, 0x0f, 0x1f, 0x3f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff,
                0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x1f, 0x0f, 0x0f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1c,
                0x1c, 0x00, 0x80, 0x91, 0x91, 0x03, 0x00, 0x38, 0x38, 0x78, 0xf8, 0xf8, 0xf8, 0xf0, 0xf0, 0xf0,
                0xf0, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xfc,
                0xf8, 0xf0, 0xe0, 0xe0, 0xe0, 0xf0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff, 0xff, 0x1f, 0x06, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x81, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff,
                0xff, 0xfe, 0xf0, 0xe0, 0xc3, 0xc7, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f,
                0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f,
                0x8f, 0x8f, 0xc7, 0xc3, 0xe0, 0xf0, 0xfc, 0xff

        };

//EEPROM





void messageArrived(MQTT::MessageData &md) {
    MQTT::Message &message = md.message;
    char buffer[100];//28 when false, 30 when true
    std::cout << "Message arrived: qos " << message.qos << ", retained " << message.retained << ", dup " << message.dup << ", packet id " << message.id << std::endl;
    std::cout << "Payload " << message.payload << std::endl;
    char * payload = (char*) message.payload;
    if (payload[0] == '{') {
        std::cout << "=======================" << std::endl;
        for (int i = 0; i< 100; i++) {
            char current_char = payload[i];
            buffer[i] = payload[i];
            if (current_char == '}') {
                break;
            }
        }
        //If auto mode, set pressure with web-UI messages
        if (buffer[2] == 'a' && buffer[3] == 'u' && buffer[4] == 't' && buffer[5] == 'o') {
            if (buffer[9] == 't') {
                mfic->setAuto(1);
                if (buffer[28] == '}') {
                    mfic->setPressure((int)buffer[27]-48);
                } else if (buffer[29] == '}') {
                    mfic->setPressure(((int)buffer[27]-48)*10 + ((int)buffer[28]-48)); // *10 for scaling to web-UI
                } else if (buffer[30] == '}') {
                    mfic->setPressure(((int)buffer[27]-48)*100 + ((int)buffer[28]-48)*10 + ((int)buffer[29]-48));
                }
            }
            if (buffer[9] == 'f'){
                mfic->setAuto(0);
                if (buffer[26] == '}') {
                    mfic->setSpeed(((int)buffer[25]-48)*10);
                } else if (buffer[27] == '}') {
                    mfic->setSpeed(((int)buffer[25]-48)*100 + ((int)buffer[26]-48)*10);
                } else if (buffer[28] == '}') {
                    mfic->setSpeed(1000);
                }
            }
        }
    }
}
// Interrupt handler for rotary encoder
static void a_interrupt_handler(uint gpio, uint32_t event) {
    if(gpio == ROT_A_PIN && gpio_get(ROT_B_PIN)) mfic->setSpeed(-25);
    else mfic->setSpeed(25);
}



int main() {
    Sensor sens = Sensor();
    static const char *topic = "g09/controller/settings"; //topics
    static const char *topic1 = "g09/controller/status"; //topics
    std::vector<int> readings;
    std::weak_ptr<Controller> conny = mfic;
    Handler handy = Handler(conny);
    Target target = Target("SmartIotMQTT","SmartIot","192.168.1.10",1883);
    // Initialize LED pin
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);

    //initialize rotary encoder
    gpio_init(ROT_A_PIN);
    gpio_init(ROT_B_PIN);
    gpio_init(ROT_SW_PIN);

    gpio_set_dir(ROT_A_PIN, GPIO_IN);
    gpio_set_dir(ROT_B_PIN, GPIO_IN);
    gpio_set_dir(ROT_SW_PIN, GPIO_IN);
    gpio_set_pulls(ROT_SW_PIN, true, false);  // Enable pull-up for the switch

    gpio_set_irq_enabled_with_callback(ROT_A_PIN, GPIO_IRQ_EDGE_RISE, true, &a_interrupt_handler);

    // Initialize chosen serial port
    stdio_init_all();

    std::cout << std::endl << "Boot" << std::endl;

    // I2C is "open drain",
    // pull-ups to keep signal high when no data is being sent
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(14, GPIO_FUNC_I2C); // the display has external pull-ups
    gpio_set_function(15, GPIO_FUNC_I2C); // the display has external pull-ups
    i2c_init(i2c0, 100 * 1000); // EEPROM
    gpio_set_function(16, GPIO_FUNC_I2C);
    gpio_set_function(17, GPIO_FUNC_I2C);
    gpio_pull_up(16);
    gpio_pull_up(17);
    ssd1306 display(i2c1);
    display.fill(0);
    display.text("Welcome!", 0, 0);
    mono_vlsb rb(ventilator40x40, 40, 40);
    display.blit(rb, 2, 20);
    display.text("Credits:", 45, 20, 1);
    display.text(">Andrea", 50, 30, 1);
    display.text(">Daniel", 50, 40, 1);
    display.text(">Jeferson", 50, 50, 1);
    display.show();
    // load settings from EEPROM
    handy.load();


    uint16_t pressure_measurement = 0;

    //IPStack ipstack("SSID", "PASSWORD"); // example
    IPStack ipstack(target.getSSID().data(), target.getPassword().data()); // example
    auto client = MQTT::Client<IPStack, Countdown, 250>(ipstack); //added 200, for max size of the MQTT-mssg

    int rc = ipstack.connect(target.getHostname().data(), target.getPort()); // mqtt server ip and port, SmartIotMQTT server IP: 192.168.1.10
    if (rc != 1) {
        std::cout << "rc from TCP connect is " << rc << std::endl;
    }
    std::cout << "MQTT connecting" << std::endl;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char *) "PicoW-G9";
    rc = client.connect(data);
    if (rc != 0) {
        std::cout << "rc from MQTT connect is " << rc << std::endl;
        while (true) {
            tight_loop_contents();
        }
    }
    std::cout << "MQTT connected" << std::endl;

    // We subscribe QoS2. Messages sent with lower QoS will be delivered using the QoS they were sent with
    rc = client.subscribe(topic, MQTT::QOS2, messageArrived);
    if (rc != 0) {
        std::cout << "rc from MQTT subscribe is " << rc << std::endl;
    }
    std::cout << "MQTT subscribed" << std::endl;

    auto mqtt_send = make_timeout_time_ms(2000);
    int msg_count = 0;


    auto uart{std::make_shared<PicoUart>(UART_NR, UART_TX_PIN, UART_RX_PIN, BAUD_RATE, STOP_BITS)};
    auto rtu_client{std::make_shared<ModbusClient>(uart)};
    ModbusRegister rh(rtu_client, 241, 256);
    ModbusRegister temp(rtu_client, 241, 257);
    ModbusRegister co2(rtu_client, 240, 256);
    auto modbus_poll = make_timeout_time_ms(1500);
    auto display_poll = make_timeout_time_ms(10);
    ModbusRegister produal(rtu_client, 1, 0); // fan
    auto temperature = static_cast<float>(temp.read()) / 10.0;
    auto r_humidity = static_cast<float>(rh.read()) / 10.0;
    auto carbon_dioxide = static_cast<float>(co2.read()); //static_cast<float>(co2.read())

    while (true) {
        if (time_reached(modbus_poll)) {
            gpio_put(led_pin, !gpio_get(led_pin)); // toggle  led
            modbus_poll = delayed_by_ms(modbus_poll, 3000);
            temperature = static_cast<float>(temp.read()) / 10.0;
            r_humidity = static_cast<float>(rh.read()) / 10.0;
            carbon_dioxide = static_cast<float>(co2.read());
            printf("RH=%5.1f%%\n", r_humidity);
            printf("TEMP=%5.1f C\n", temperature);
            printf("CO2=%5.1f ppm\n", carbon_dioxide);
            pressure_measurement = sens.measure();
            if (mfic->getAuto() == 1 && readings.size() < 20) readings.push_back(pressure_measurement);
            else if (mfic->getAuto() == 1 && readings.size() == 20) {
                readings.erase(readings.begin());
                readings.push_back(pressure_measurement);
            } else readings.clear();//if
            std::cout << "Pressure=" << pressure_measurement << std::endl;
            std::cout << "Fan speed=" << (mfic->getSpeed() / 10) << std::endl;
            std::cout << "AUTO mode=" << (mfic->getAuto()) << std::endl;
            if (mfic->getAuto()) {
                if (pressure_measurement < mfic->getPressure() - 1) {
                    mfic->setSpeed((mfic->getPressure() - pressure_measurement)*6); //*6
                }
                if (pressure_measurement > mfic->getPressure() + 1) {
                    mfic->setSpeed(-(pressure_measurement - mfic->getPressure())*6); //*6
                }
            }
            handy.save();
        }
        produal.write(mfic->getSpeed());

        //Show data on display
        if (time_reached(display_poll)) {
            display_poll = delayed_by_ms(display_poll, 10);
            display.fill(0);
            //Slide bar for fan speed
            display.rect(0, 0, int(mfic->getSpeed()*0.128),5, 1,true);

            //Measurements
            char tempString[10];
            char humString[10];
            char co2String[10];
            char pressureString[10];
            char fanspeedString[20];
            char error[24];
            sprintf(tempString, "T:%.1f", temperature);
            sprintf(humString, "RH :%.1f", r_humidity);
            sprintf(co2String, "CO2:%.1f", carbon_dioxide);
            sprintf(pressureString, "P:%u", pressure_measurement);
            sprintf(fanspeedString, "Fan Speed:%d%%", mfic->getSpeed() / 10);
            sprintf(error,"P not set in 1m");
            display.text(tempString, 0, 30, 1);
            display.text(humString, 55, 30, 1);
            display.text(co2String, 55, 40, 1);
            display.text(pressureString, 0, 40, 1);
            if (mfic->getAuto() == 1 && (!(mfic->getPressure() < (*readings.end() - 5) || mfic->getPressure()  > (*readings.end() + 5)))) {
                display.rect(0,49,128,10,1, true);
                display.text(error, 0, 50, 0);
            }
            if(mfic->getAuto()){
                display.text("[AUTO]", 45, 15, 1);
            } else {
                display.text(fanspeedString, 10, 15, 1);
            }
            display.show();
        }

        if (time_reached(mqtt_send)) {
            mqtt_send = delayed_by_ms(mqtt_send, 2000);
            if (!client.isConnected()) {
                std::cout << "Not connected..." << std::endl;
                rc = client.connect(data);
                if (rc != 0) {
                    std::cout << "rc from MQTT connect is " << rc;
                }
            }
            char buf[250]; //raised buffer size
            rc = 0;
            MQTT::Message message{};
            message.retained = false;
            message.dup = false;
            message.payload = (void *) buf;
            sprintf(buf,"{\"pressure\": %u, "
                        "\"auto\": %s, "
                        "\"speed\": %d, "
                        "\"co2\": %.1f, "
                        "\"rh\": %.1f, "
                        "\"temp\": %.1f, "
                        "\"setpoint\": %d, "
                        "\"error\": %s, "
                        "\"nr\": %d "
                        "}",
                    pressure_measurement,
                    (mfic->getAuto() == 1) ? "true" : "false",
                    mfic->getSpeed() / 10,
                    carbon_dioxide,
                    r_humidity,
                    temperature,
                    (mfic->getAuto() == 1) ? mfic->getPressure() : (mfic->getSpeed() / 10),
                    (mfic->getAuto() == 1 && (!(mfic->getPressure() < (*readings.end() - 5) || mfic->getPressure() > (*readings.end() + 5))) && readings.size()==20) ? "true" : "false",
                    ++msg_count);
            std::cout << buf << std::endl;
            message.qos = MQTT::QOS2;
            message.payloadlen = strlen(buf);
            rc = client.publish(topic1, message);
            std::cout << "Publish rc=" << rc << std::endl;
        }
        cyw43_arch_poll(); // obsolete? - see below
        client.yield(100); // socket that client uses calls cyw43_arch_poll()
    }
}