/*
  Распиновка переключателя режимов работы:
    Upload to ESP8266 (WiFi)              0 0 0 0 1 1 1 0
    Upload to ATmega328 (Thermo)          0 0 1 1 0 0 0 0
    Interconnect ESP8266 and ATmega328    1 1 0 0 0 0 0 0
*/
#include <MAX6675_Thermocouple.h>
#include <RunningMedian.h>
#include <ArduinoJson.h>

const int measurement_delay = 250;
const int measurement_count = 120;
unsigned long measurement_epoch = 0;
unsigned long measurement_prev_millis = 0;

RunningMedian temperatures(measurement_count);


const int thermo_sck_pin = 2;
const int thermo_cs_pin  = 4;
const int thermo_so_pin  = 7;

MAX6675_Thermocouple thermocouple(thermo_sck_pin, thermo_cs_pin, thermo_so_pin);


void setup() {
  Serial.begin(57600);
}


void loop() {
  // Adding measurement to buffer.
  delay(measurement_delay);
  temperatures.add(thermocouple.readCelsius());
  if (temperatures.getCount() < temperatures.getSize()) {
    return;
  }

  // Updating epoch on clock overflow.
  unsigned long clock_time = millis();
  if (clock_time < measurement_prev_millis) {
    measurement_epoch++;
  }
  measurement_prev_millis = clock_time;

  // Building JSON payload.
  StaticJsonDocument<128> doc;
  doc["epoch"] = measurement_epoch;
  doc["clock"] = clock_time;
  doc["median_temp"] = temperatures.getMedian();

  // Sending payload to esp8266 chip via serial.
  char output[128];
  serializeJson(doc, output);
  Serial.println(output);

  // Restarting measurements.
  temperatures.clear();
}
