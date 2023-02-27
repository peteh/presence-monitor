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
    /**
     * @brief Updates the sensor data, must be run as fast as possible
     *
     * @return true if data has been updated for the sensor in this cycle
     * @return false if data has not been updated in this cycle
     */
    bool loop();


    bool hasMotion();
    bool hasOccupancy();

    void sendRequest();

    /**
     * @brief Set the Motion Threshold object
     * 
     * @param threshold minimal signal strength to report detected motion
     */
    void setMotionThreshold(uint16_t threshold);

    /**
     * @brief Set the Occupancy Threshold object
     * 
     * @param threshold minimal signal strength to report detected occupancy
     */
    void setOccupancyThreshold(uint16_t threshold);

    /**
     * @brief Save currently set Motion Threshold (th1) and occupancy threshold (th2) in the sensor.
     *
     */
    void saveConfig();

    void setMotionTimeout(uint16_t timeout)
    {
        m_motionTimeoutS = timeout;
    }

    void setOccupancyTimeout(uint16_t timeout)
    {
        m_occupancyTimeoutS = timeout;
    }

private:
    void processLine(char *data);

    void updateMotion(int beam, int signal);
    void updateOccupancy(int beam, int signal);

    char m_buffer[200];
    unsigned int m_index = 0;
    unsigned long m_lastOccupancy;
    unsigned long m_lastMotion;

    uint16_t m_motionTimeoutS = 10;
    uint16_t m_occupancyTimeoutS = 10;

    uint16_t m_motionThreshold = 200;
    uint16_t m_occupancyThreshold = 200;

    SoftwareSerial m_serial;
};
