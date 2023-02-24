#include <Arduino.h>
#include <SoftwareSerial.h>
#include <esplog.h>
#include "Sensor.h"

Sensor::Sensor(uint8_t RX, uint8_t TX)
    : m_serial(RX, TX)
{
}

void Sensor::begin()
{
    m_serial.begin(115200);
}

void Sensor::sendRequest()
{
    m_serial.printf("get_all\n");
    while(!loop());
}

void Sensor::setMotionThreshold(uint16_t threshold)
{
    m_serial.printf("th1=%d\n", threshold);
    while(!loop());
}

void Sensor::setOccupancyThreshold(uint16_t threshold)
{
    m_serial.printf("th2=%d\n", threshold);
    while(!loop());
}

void Sensor::processLine(char *data)
{
    char *pch;
    // Serial.printf("Splitting string \"%s\" into tokens:\n", data);
    pch = strtok(data, " ");
    if (pch == NULL)
    {
        log_error("Received empty line");
        return;
    }
    else if (strcmp(pch, "null") == 0)
    {
        log_debug("null header received");
        // No data
    }
    else if (strcmp(pch, "occ,") == 0)
    {
        pch = strtok(NULL, " ");
        if (pch == NULL)
        {
            log_error("Failed to parse first argument from occ");
            return;
        }
        int beam = atoi(pch);

        pch = strtok(NULL, " ");
        if (pch == NULL)
        {
            log_error("Failed to parse second argument from occ");
            return;
        }
        int signalStrength = atoi(pch);
        updateOccupancy(beam, signalStrength);
    }
    else if (strcmp(pch, "mov,") == 0)
    {
        pch = strtok(NULL, " ");
        if (pch == NULL)
        {
            log_error("Failed to parse first argument from mov");
            return;
        }
        int beam = atoi(pch);

        pch = strtok(NULL, " ");
        if (pch == NULL)
        {
            log_error("Failed to parse second argument from mov");
            return;
        }
        int signalStrength = atoi(pch);
        updateMotion(beam, signalStrength);
    }
    else if (strcmp(pch, "th1") == 0)
    {
        pch = strtok(NULL, " ");
        if (pch == NULL)
        {
            log_error("Failed to parse first argument from th1");
            return;
        }
        pch = strtok(NULL, " ");
        if (pch == NULL)
        {
            log_error("Failed to parse second argument from th1");
            return;
        }
        int motionThreshold = atoi(pch);
        log_info("TH1 (motion) is %d", motionThreshold);
    }
    else if (strcmp(pch, "th2") == 0)
    {
        pch = strtok(NULL, " ");
        if (pch == NULL)
        {
            log_error("Failed to parse first argument from th2");
            return;
        }
        pch = strtok(NULL, " ");
        if (pch == NULL)
        {
            log_error("Failed to parse second argument from th2");
            return;
        }
        int occupancyThreshold = atoi(pch);
        log_info("TH2 (occupancy) is %d", occupancyThreshold);
    }
    else
    {
        log_error("Unknown header: %s, %8x\n", pch, pch);
        while (pch != NULL)
        {
            log_error(" data: %s", pch);
            pch = strtok(NULL, " ");
            
        }
    }
}

bool Sensor::loop()
{
    // used to force exit of loop after certain amount of characters
    uint8_t maxData = 200;
    char c;
    while (m_serial.available() && maxData > 0)
    {
        c = m_serial.read();
        maxData--;
        if (c == '\n')
        {
            m_buffer[m_index] = 0;
            processLine(m_buffer);
            m_index = 0;
            m_buffer[m_index] = 0;
            return true;
        }
        else
        {
            m_buffer[m_index++] = c;
        }
    }
    return false;
}

void Sensor::updateMotion(int beam, int signal)
{
    log_debug("Motion(%d, %d)", beam, signal);
    m_lastMotion = millis();
}

void Sensor::updateOccupancy(int beam, int signal)
{
    log_debug("Occupancy(%d, %d)", beam, signal);
    m_lastOccupancy = millis();
}

bool Sensor::hasMotion()
{
    unsigned long threshold = 10000;
    unsigned long currentTime = millis();
    return (currentTime - m_lastMotion < threshold);
}

bool Sensor::hasOccupancy()
{
    unsigned long threshold = 10000;
    unsigned long currentTime = millis();
    return (hasMotion() || currentTime - m_lastOccupancy < threshold);
}