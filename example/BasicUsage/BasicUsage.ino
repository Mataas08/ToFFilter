/*
  BasicUsage.ino - Example for ToFFilter library
  Author: Mateusz Niedobecki
  Date: 2025-08-29

  This example shows how to use ToFFilter with the VL53L0X sensor.
  It demonstrates:
    - Sensor initialization
    - ToFFilter configuration
    - Filtering of raw measurements
    - Printing both raw and filtered results to Serial Monitor

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
    XSHUT -> Digital pin (e.g. D2, optional)
    VIN   -> 5V (if module has regulator) or 3.3V (bare sensor)
    GND   -> GND
  ---------------------------------------------------------------
*/

#include <Wire.h>
#include <VL53L0X.h>
#include <ToFFilter.h>

// --- Global objects ---
VL53L0X sensor;       // raw ToF sensor
ToFFilter tofFilter;  // filtering wrapper


void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("=== ToFFilter + VL53L0X Example ===");

  // -----------------------------------------------------------
  // I2C initialization - depends on board type
  // -----------------------------------------------------------
  #if defined(ARDUINO_AVR_UNO)
    // Arduino UNO: SDA=A4, SCL=A5 (hardware fixed)
    Wire.begin();
    const int XSHUT_PIN = 2; // use D2 as reset (optional)

  #elif defined(ARDUINO_ARCH_ESP32)
    // ESP32-C3: we can select custom pins
    Wire.begin(7, 6); // SDA=7, SCL=6
    const int XSHUT_PIN = 4;

  #else
    #error "Board not supported in this example"
  #endif

  // -----------------------------------------------------------
  // Optional reset pin (XSHUT)
  // -----------------------------------------------------------
  pinMode(XSHUT_PIN, OUTPUT);
  digitalWrite(XSHUT_PIN, HIGH); // keep sensor powered
  delay(10);

  // -----------------------------------------------------------
  // Initialize VL53L0X sensor
  // -----------------------------------------------------------
  if (!sensor.init()) {
    Serial.println("❌ Failed to detect VL53L0X sensor!");
    while (1) {
      delay(1000);
    }
  }

  // Set measurement timing budget
  // Default = 33 ms (≈30 Hz), good tradeoff between speed and accuracy
  sensor.setMeasurementTimingBudget(33000);

  // Start continuous measurements
  sensor.startContinuous(0);

  Serial.println("✅ VL53L0X initialized");

  // -----------------------------------------------------------
  // Configure ToFFilter
  // -----------------------------------------------------------
  tofFilter.setOffset(10);              // calibration offset in mm
  tofFilter.setRangeLimits(15, 2000);   // valid range: 15 mm – 2 m
  tofFilter.setPublishInterval(200);    // output every ≥200 ms
  tofFilter.setAlphaLimits(0.02f, 0.6f);// smoothing aggressiveness
  tofFilter.setDeadband(2);             // ignore ±2 mm jitter
  tofFilter.setDeltaNorm(60);           // normalization factor

  Serial.println("✅ ToFFilter configured");
}


void loop() {
  // -----------------------------------------------------------
  // Read raw value from VL53L0X
  // -----------------------------------------------------------
  int raw = sensor.readRangeContinuousMillimeters();

  // Pass through filter
  double filtered = tofFilter.filter(raw);

  // -----------------------------------------------------------
  // Print results
  // -----------------------------------------------------------
  Serial.print("Raw [mm]: ");
  Serial.print(raw);

  Serial.print("   |   Filtered [m]: ");
  if (isnan(filtered)) {
    Serial.println("NAN (out of range)");
  } else {
    Serial.println(filtered, 3); // 3 decimals = mm precision
  }

  delay(20); // small delay to not flood Serial Monitor
}
