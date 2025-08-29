# ToFFilter Library

Adaptive filtering library for Time-of-Flight (ToF) distance sensors (VL53L0X, VL53L1X, etc.).  
It stabilizes raw measurements using:  
- **Median filter** (removes outliers)  
- **Deadband** (ignores micro-jitter)  
- **Adaptive EMA** (exponential moving average) with **log-sigmoid dynamic α**  

<img width="1383" height="912" alt="Figure" src="https://github.com/user-attachments/assets/1172998a-da6d-4f2c-958a-fb1243123eab" />

---

## Features
- Median filter (3 samples) → removes spikes.  
- Deadband → ignores very small changes.  
- Adaptive EMA → reacts quickly to large changes, smooths noise when stable.  
- Configurable parameters: offset, min/max range, publish interval, alpha limits, deadband, delta normalization.  
- Returns distance in **meters** or `NAN` if out of range.  

---

## Installation
1. Copy this folder into your Arduino `libraries/` directory.  
2. Restart Arduino IDE.  
3. Include with:  
   ```cpp
   #include <ToFFilter.h>
   ```

---

## Usage Example

```cpp
#include <Wire.h>
#include <VL53L0X.h>
#include <ToFFilter.h>

VL53L0X sensor;
ToFFilter tofFilter;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  sensor.init();
  sensor.setMeasurementTimingBudget(33000); // default mode
  sensor.startContinuous(0);

  // configure filter
  tofFilter.setOffset(10);
  tofFilter.setRangeLimits(15, 2000);
  tofFilter.setPublishInterval(200);
  tofFilter.setAlphaLimits(0.02f, 0.6f);
  tofFilter.setDeadband(2);
  tofFilter.setDeltaNorm(60);
}

void loop() {
  int raw = sensor.readRangeContinuousMillimeters();
  double filtered = tofFilter.filter(raw);

  if (!isnan(filtered)) {
    Serial.print("Distance: ");
    Serial.print(filtered, 3);
    Serial.println(" m");
  } else {
    Serial.println("Out of range");
  }

  delay(10);
}
```

---

## Configuration Functions

### `setOffset(int mm)`
- **Purpose**: Apply calibration offset to all measurements.  
- **Default**: `10 mm`  
- **Usage**:  
  ```cpp
  tofFilter.setOffset(15); // add calibration offset
  ```

---

### `setRangeLimits(int minMm, int maxMm)`
- **Purpose**: Define the valid measurement range.  
- Values below `minMm` return `0.0`.  
- Values above `maxMm` return `NAN`.  
- **Default**: `15 mm – 2000 mm`  
- **Usage**:  
  ```cpp
  tofFilter.setRangeLimits(20, 2500);
  ```

---

### `setPublishInterval(unsigned long ms)`
- **Purpose**: Minimum interval between published values.  
- Ensures SUPLA/cloud is not flooded with too many updates.  
- **Default**: `200 ms`  
- **Usage**:  
  ```cpp
  tofFilter.setPublishInterval(500); // 2 Hz max
  ```

---

### `setAlphaLimits(float minAlpha, float maxAlpha)`
- **Purpose**: Configure smoothing aggressiveness.  
- `minAlpha` → applied when object is stable / far away.  
- `maxAlpha` → applied when object moves fast / close.  
- **Default**: `0.02 – 0.6`  
- **Usage**:  
  ```cpp
  tofFilter.setAlphaLimits(0.01f, 0.8f);
  ```

---

### `setDeadband(int mm)`
- **Purpose**: Ignore tiny changes within ±deadband mm.  
- Prevents “jumping” between e.g. 16.0 ↔ 16.1 cm.  
- **Default**: `2 mm`  
- **Usage**:  
  ```cpp
  tofFilter.setDeadband(5); // ignore ±5 mm noise
  ```

---

### `setDeltaNorm(int mm)`
- **Purpose**: Scale factor for deltaFactor (sensitivity to changes).  
- Larger value = smoother filter, smaller = more reactive.  
- **Default**: `60 mm`  
- **Usage**:  
  ```cpp
  tofFilter.setDeltaNorm(80);
  ```

---

## Main Function

### `double filter(int rawMm)`
- Input: raw measurement in **millimeters** (from sensor).  
- Output: filtered value in **meters** (`double`).  
- Returns `NAN` if out of range.  
- Returns `0.0` if below minimum valid range.  

---

## Notes
- Works best with VL53L0X/VL53L1X in **continuous mode** (`startContinuous(0)`).  
- Adjust `setDeltaNorm` and `setDeadband` for your environment (indoor/outdoor, reflective surfaces).  
- Calibration: put a white target at known distance (e.g. 100 mm), measure offset, set via `setOffset()`.  

---

## Presets

If you don’t want to fine-tune every parameter, you can use ready presets:

---

### ⚡ FastResponse
Good for dynamic applications (e.g. parking sensor, motion detection).

```cpp
tofFilter.setOffset(10);
tofFilter.setRangeLimits(15, 2000);
tofFilter.setPublishInterval(100);
tofFilter.setAlphaLimits(0.05f, 0.8f);
tofFilter.setDeadband(1);
tofFilter.setDeltaNorm(40);
```

---

### 🛡️ StableLongRange
Good for stable long-distance measurements (e.g. tanks, static monitoring).

```cpp
tofFilter.setOffset(10);
tofFilter.setRangeLimits(15, 3000);
tofFilter.setPublishInterval(500);
tofFilter.setAlphaLimits(0.01f, 0.4f);
tofFilter.setDeadband(5);
tofFilter.setDeltaNorm(100);
```

---

## License
MIT License


