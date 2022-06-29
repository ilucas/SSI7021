#include <SSI7021.h>
#include <Arduino.h>

uint32_t SSI7021::_pulseMaxCycles = microsecondsToClockCycles(1000);  // 1 millisecond timeout for reading pulses from DHT sensor.

SSI7021::SSI7021(uint8_t pin) : _pin(pin) {}

SSI7021::~SSI7021() {
    pinMode(_pin, INPUT);    
}

/*
* Expect the signal line to be at the specified level for a period of time and
* return a count of loop cycles spent at that level (this cycle count can be
* used to compare the relative time of two pulses).  If more than a millisecond
* ellapses without the level changing then the call fails with a 0 response.
*
* https://github.com/adafruit/DHT-sensor-library/blob/master/DHT.cpp#L362
*/
int32_t SSI7021::expectPulse(int level) {
    int32_t count = 0;
    while (digitalRead(_pin) == level) {
        if (count++ >= (int32_t)_pulseMaxCycles) {
            return -1;  // Timeout
        }
    }
    return count;
}

/*
 *
 * 1) MCU PULLS LOW data bus for at 500us to activate sensor
 * 2) MCU PULLS UP data bus for ~40us to ask sensor for response
 * 3) SENSOR starts sending data (LOW 40us then HIGH ~25us for "0" or ~75us for "1")
 * 4) SENSOR sends "1" start bit as a response
 * 5) SENSOR sends 16 bits (2 bytes) of a humidity with one decimal (i.e. 35.6% is sent as 356)
 * 6) SENSOR sends 16 bits (2 bytes) of a temperature with one decimal (i.e. 23.4C is sent as 234)
 * 7) SENSOR sends 8 bits (1 byte) checksum of 4 data bytes
 *
 * https://github.com/arendst/Tasmota/pull/7468
 * https://github.com/arendst/Tasmota/issues/735#issuecomment-348718383
 * https://github.com/arendst/Tasmota/blob/v12.0.2/tasmota/tasmota_xsns_sensor/xsns_06_dht.ino#L70
*/
bool SSI7021::read() {    
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delayMicroseconds(500);
    noInterrupts();
    pinMode(_pin, INPUT_PULLUP);
    delayMicroseconds(40);

    int32_t cycles[80];
    uint8_t error = 0;
    uint8_t data[5];

    //Test start bit
    if (expectPulse(LOW) == -1) {
        error = 1;
    } else if (expectPulse(HIGH) == -1) {
        error = 1;
    } else {
        //Read data bits
        for (uint32_t i = 0; i < 80; i += 2) {
            cycles[i]   = expectPulse(LOW);
            cycles[i+1] = expectPulse(HIGH);
        }
    }

    interrupts();
    
    if (error) {
        // probably the sensor is connected in the wrong pin or not connected at all
        return false;
    }

    // Get data
    for (uint32_t i = 0; i < 40; ++i) {
        int32_t lowCycles  = cycles[2*i];
        int32_t highCycles = cycles[2*i+1];

        if ((lowCycles == -1) || (highCycles == -1)) {
            return false;
        }

        data[i/8] <<= 1;
        if (highCycles > lowCycles) {
            data[i / 8] |= 1;
        }
    }

    //Compute checksum
    uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    if (data[4] != checksum) {
        // Serial.println(data[4]);
        // Serial.println(checksum);
        // Serial.println(F("checksum failed"));
        return false;
    }

    // Decode
    auto humidity = constrain(((data[0] << 8) | data[1]) * 0.1f, 0, 100);

    int16_t temp16 = data[2] << 8 | data[3]; // case 1 : signed 16 bits
    if ((data[2] & 0xF0) == 0x80) {          // case 2 : negative when high nibble = 0x80
        temp16 = -(0xFFF & temp16);
    }
    auto temperature = 0.1f * temp16;

    _data.t = temperature;
    _data.h = humidity;

    // Serial.print(F("Tmp: "));
    // Serial.print(temperature);
    // Serial.print(F("c Humidity: "));
    // Serial.print(humidity);
    // Serial.println("%");

    return true;
}
