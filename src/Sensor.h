#pragma once
#include <Arduino.h>
/***
 * Supports HLK-LD1115H
*/
class Sensor
{
public:
    Sensor(uint8_t RX, uint8_t TX);

    void begin();

    bool loop();
    bool hasMotion();
    bool hasOccupancy();
    void sendRequest();

    void setMotionThreshold(uint16_t threshold);
    void setOccupancyThreshold(uint16_t threshold);

private:
    void processLine(char *data);

    void updateMotion(int beam, int signal);
    void updateOccupancy(int beam, int signal);

    char m_buffer[200];
    unsigned int m_index = 0;
    unsigned long m_lastOccupancy;
    unsigned long m_lastMotion;

    SoftwareSerial m_serial;
};
