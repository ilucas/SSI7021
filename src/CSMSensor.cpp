#include <CSMSensor.h>
#include <Arduino.h>

static const PROGMEM int DryValue = 530;
static const PROGMEM int WetValue = 250;

CSMSensor::CSMSensor(uint8_t pin) : _pin(pin) {}

void CSMSensor::read() {
    int rawValue = analogRead(_pin);
    int soilmoistureRawPercent = map(rawValue, DryValue, WetValue, 0, 100);
    _value = constrain(soilmoistureRawPercent, 0, 100);
}
