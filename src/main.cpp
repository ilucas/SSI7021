#include <Arduino.h>
#include <TaskScheduler.h>
#include <ArduinoJson.h>

#include "SSI7021.h"
#include "CSMSensor.h"

static const unsigned long TaskInverval PROGMEM = 10 * 1000; //secs

SSI7021 th_sensor(12);
void th_callback();
Task th_task(TaskInverval, TASK_FOREVER, &th_callback);

CSMSensor soil_sensor(A0);
void soil_callback();
Task soil_task(TaskInverval, TASK_FOREVER, &soil_callback);

void publish_callback();
Task publish_task(TaskInverval, TASK_FOREVER, &publish_callback);

Scheduler runner;

void setup()
{
    Serial.begin(115200);

    runner.init();

    runner.addTask(th_task);
    runner.addTask(soil_task);
    runner.addTask(publish_task);

    delay(500);
    th_task.enable();
    soil_task.enable();
    publish_task.enable();
}

void loop()
{
    runner.execute();
}

void th_callback()
{
    th_sensor.read();
}

void soil_callback() 
{
    soil_sensor.read();
}

void publish_callback() 
{
    StaticJsonDocument<24> json;

    auto th_data = th_sensor.getData();

    json["t"] = th_data.t;
    json["h"] = th_data.h;
    json["s"] = soil_sensor.getValue();

    serializeJson(json, Serial);
    Serial.println("");
}
