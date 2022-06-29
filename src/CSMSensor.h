#pragma once
#include <Arduino.h>

class CSMSensor {
    public:
        CSMSensor(uint8_t pin);
        void read();
        int getValue() const { return _value; };

    private:
        uint8_t _pin;
        int _value;
};
