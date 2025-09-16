/*
  FullExample.ino - Full-featured example for ToFFilter library
  Author: Mateusz Niedobecki
  Date: 2025-09-16

  Demonstrates:
    - Sensor initialization
    - Using ToFFilter with ALL optional filters enabled:
      * Median filter (always ON)
      * Offset calibration
      * Range validation
      * Publish interval
      * Alpha/EMA adaptive smoothing
      * Deadband
      * DeltaNorm (dynamic response scaling)
      * Stability lock (freeze when stable)
      * Percent filter (ignore small relative changes at long range)

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

  Serial.println("=== ToFFilter Full Example ===");

  // -----------------------------------------------------------
  // I2C initialization - depends on board type
  // -----------------------------------------------------------
  #if defined(ARDUINO_AVR_UNO)
    // Arduino UNO: SDA=A4, SCL=A5
    Wire.begin();
    const int XSHUT_PIN = 2;

  #elif defined(ARDUINO_ARCH_ESP32)
    // ESP32-C3: custom pins
    Wire.begin(7, 6);
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

  // --- Configure ToFFilter ---
  tofFilter.setOffset(10);                  // calibration offset
  tofFilter.setRangeLimits(15, 4000);       // valid range
  tofFilter.setPublishInterval(200);        // publish interval

  // Optional filters - ALL enabled
  tofFilter.setAlpha(true, 0.015f, 0.75f);      // adaptive EMA
  tofFilter.setDeadband(true, 5);               // ignore ±5 mm
  tofFilter.setDeltaNorm(true, 60);             // scaling factor
  tofFilter.setStability(true, 30, 3000);       // freeze after 3s stable
  tofFilter.setPercentFilter(true, 0.1f, 2000); // 10% above 2 m

  Serial.println("ToFFilter configured (all filters ON)");
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
