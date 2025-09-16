/*
  BasicExample.ino - Minimal example for ToFFilter library
  Author: Mateusz Niedobecki
  Date: 2025-09-16

  Demonstrates:
    - Sensor initialization
    - Using ToFFilter with default filters:
      * Median filter (always ON)
      * Offset (default: 10 mm)
      * Range limits (default: 15–2000 mm)
      * Publish interval (default: 200 ms)

  ---------------------------------------------------------------
  Wiring (ESP32-C3):
    SDA   -> GPIO7
    SCL   -> GPIO6
    XSHUT -> GPIO4 (optional, can be tied to 3.3V if unused)
    VIN   -> 3.3V
    GND   -> GND

  Wiring (Arduino UNO):
    SDA   -> A4
    SCL   -> A5
    XSHUT -> D2 (optional, can be tied to VCC if unused)
    VIN   -> 5V (if module has regulator) or 3.3V
    GND   -> GND
  ---------------------------------------------------------------
*/

#include <Wire.h>
#include <VL53L0X.h>
#include <ToFFilter.h>

VL53L0X sensor;
ToFFilter tofFilter;

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("=== ToFFilter Basic Example ===");

  // -----------------------------------------------------------
  // I2C initialization - depends on board type
  // -----------------------------------------------------------
  #if defined(ARDUINO_AVR_UNO)
    // Arduino UNO: SDA=A4, SCL=A5 (hardware fixed)
    Wire.begin();
    const int XSHUT_PIN = 2; // optional reset pin

  #elif defined(ARDUINO_ARCH_ESP32)
    // ESP32-C3: custom pins
    Wire.begin(7, 6); // SDA=7, SCL=6
    const int XSHUT_PIN = 4;

  #else
    #error "Board not supported in this example"
  #endif

  // XSHUT pin (optional)
  pinMode(XSHUT_PIN, OUTPUT);
  digitalWrite(XSHUT_PIN, HIGH);
  delay(10);

  if (!sensor.init()) {
    Serial.println("Failed to detect VL53L0X!");
    while (1) delay(1000);
  }

  sensor.setMeasurementTimingBudget(33000);
  sensor.startContinuous(0);

  Serial.println("VL53L0X initialized with ToFFilter defaults");
}

void loop() {
  int raw = sensor.readRangeContinuousMillimeters();
  double filtered = tofFilter.filter(raw);

  Serial.print("Raw [mm]: ");
  Serial.print(raw);

  Serial.print("   |   Filtered [m]: ");
  if (isnan(filtered)) {
    Serial.println("NAN (out of range)");
  } else {
    Serial.println(filtered, 3);
  }

  delay(50);
}
