#pragma once
#include <Arduino.h>

class SSI7021 {
    struct Data {
        float t = NAN;
        float h = NAN;
    };

    public:
        SSI7021(uint8_t pin);
        ~SSI7021();
        bool read();
        Data getData() const { return _data; };

    private:
        static uint32_t _pulseMaxCycles;
        uint8_t _pin;
        Data _data;
        int32_t expectPulse(int level);
};
