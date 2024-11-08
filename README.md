# Ventilation Controller System

This project implements a ventilation controller system for controlling and monitoring the ventilation fan in both manual and automatic modes, accessible through a local user interface or remotely through MQTT.

![image](https://github.com/user-attachments/assets/8a00c4cb-906c-47d2-8210-99ff8ec9ca9f)

### Project Overview

The controller operates in two modes:
- **Manual Mode**: The user sets the fan speed (0–100%) from either the local UI or via MQTT.
- **Automatic Mode**: The user specifies a target pressure level in the ventilation duct, and the controller adjusts the fan speed to maintain that pressure.

### Hardware Components
- **Produal MIO 12-V**: Controls the fan speed with a 0–10V output, where 0V is off (0%) and 10V is full speed (100%).
- **Vaisala GMP252 CO₂ Sensor**: Measures CO₂ levels.
- **Vaisala HMP60 Sensor**: Monitors temperature and relative humidity.
- **Sensirion SDP610-120Pa Pressure Sensor**: Measures the differential pressure between the room and the duct.
- **EEPROM**: Stores device data persistently.

### Software Features
- **Manual and Automatic Modes**: Operated via a local UI (rotary encoder, buttons, and OLED display) or remotely via MQTT messages.
- **MQTT Integration**: Sends status updates and receives control messages on topics `controller/status` and `controller/settings`.
- **Status Messages**: Reports the controller state, sensor readings, and error states, formatted as JSON for easy integration with other systems.
- **Persistent Storage**: Allows storage of network credentials and MQTT broker IP settings.

### Testing and Simulation
Miniature test system consists of similar components as the “big” test system with the exception that some of the components are simulated by Arduino software.

![image](https://github.com/user-attachments/assets/2313890e-9913-437a-bda7-6efc44339e4d)
![image](https://github.com/user-attachments/assets/56df3bd7-14f5-4f3f-821d-273ea0fabb17)
