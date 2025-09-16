# ToFFilter Library

Adaptive filtering library for Time-of-Flight (ToF) distance sensors (VL53L0X, VL53L1X, etc.).  
It allows flexible noise reduction and stabilization of raw distance data.  

The library includes several optional filters (all disabled by default).  
You can enable each one individually if needed.

<img width="1383" height="912" alt="Figure" src="https://github.com/user-attachments/assets/1172998a-da6d-4f2c-958a-fb1243123eab" />

---

## Features
- **Median filter (always ON)** → removes single-sample spikes.  
- **Offset calibration** → correct sensor bias.  
- **Range validation** → clamp or reject invalid values.  
- **Publish interval** → ensures stable update rate.  
- **Optional filters (disabled by default)**:  
  - Alpha/EMA smoothing (with log-sigmoid adaptive curve).  
  - Deadband (ignore tiny variations).  
  - DeltaNorm (normalize reactivity to changes).  
  - Stability lock (freeze value when no motion is detected).  
  - Percent filter (ignore small relative changes at long distances).  
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

## Examples

Examples are included in the `examples/` folder

Open them directly from **Arduino IDE → File → Examples → ToFFilter**.  

---

## Configuration Functions

### `setOffset(int mm)`
- **Purpose**: Constant calibration offset.  
- Always applied.  
- **Default**: `10 mm`.  
- **Usage**:
  ```cpp
  tofFilter.setOffset(15);
  ```

---

### `setRangeLimits(int minMm, int maxMm)`
- **Purpose**: Define valid measurement range.  
- Below `minMm` → returns `0.0`.  
- Above `maxMm` → returns `NAN`.  
- **Default**: `15 – 2000 mm`.  
- **Usage**:
  ```cpp
  tofFilter.setRangeLimits(20, 3000);
  ```

---

### `setPublishInterval(unsigned long ms)`
- **Purpose**: Minimum time between published results.  
- Prevents flooding cloud/serial with too many updates.  
- **Default**: `200 ms`.  
- **Usage**:
  ```cpp
  tofFilter.setPublishInterval(500); // 2 Hz max
  ```

---

### `setAlpha(bool enabled, float minAlpha, float maxAlpha)`
- **Purpose**: Enable/disable EMA smoothing.  
- Disabled by default.  
- `minAlpha` → small updates when stable.  
- `maxAlpha` → fast updates on rapid change.  
- **Usage**:
  ```cpp
  tofFilter.setAlpha(true, 0.015f, 0.75f); // enable EMA
  tofFilter.setAlpha(false);               // completely disable
  ```

---

### `setDeadband(bool enabled, int mm)`
- **Purpose**: Ignore tiny changes within ±deadband mm.  
- Disabled by default.  
- **Usage**:
  ```cpp
  tofFilter.setDeadband(true, 30); // enable
  tofFilter.setDeadband(false);    // disable
  ```

---

### `setDeltaNorm(bool enabled, int mm)`
- **Purpose**: Normalize responsiveness to movement.  
- Disabled by default.  
- Larger value = smoother, smaller = more reactive.  
- **Usage**:
  ```cpp
  tofFilter.setDeltaNorm(true, 70); // enable
  tofFilter.setDeltaNorm(false);    // disable
  ```

---

### `setStability(bool enabled, int mm, unsigned long ms)`
- **Purpose**: Freeze value if changes stay below `mm` for `ms` milliseconds.  
- Disabled by default.  
- **Usage**:
  ```cpp
  tofFilter.setStability(true, 50, 3000); // enable freeze after 3s
  tofFilter.setStability(false);          // disable stability lock
  ```

---

### `setPercentFilter(bool enabled, float pct, int startMm = -1)`
- **Purpose**: Ignore small relative changes at long distances.  
- Disabled by default.  
- `pct` = relative threshold (e.g. `0.1` = 10%).  
- `startMm` = distance at which filter activates (default: half of max range).  
- **Usage**:
  ```cpp
  tofFilter.setPercentFilter(true, 0.1f, 2000); // active above 2000 mm
  tofFilter.setPercentFilter(false);            // disable
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

### ⚡ FastDynamic
Good for quick reaction (e.g. gesture sensing, parking).  
```cpp
tofFilter.setOffset(10);
tofFilter.setRangeLimits(15, 2000);
tofFilter.setPublishInterval(100);

tofFilter.setAlpha(true, 0.05f, 0.8f);
tofFilter.setDeadband(true, 2);
tofFilter.setDeltaNorm(true, 40);
```

---

### 🛡️ StableMonitoring
For tanks, room monitoring, stable targets.  
```cpp
tofFilter.setOffset(10);
tofFilter.setRangeLimits(15, 4000);
tofFilter.setPublishInterval(500);

tofFilter.setAlpha(true, 0.01f, 0.4f);
tofFilter.setDeadband(true, 10);
tofFilter.setDeltaNorm(true, 100);
tofFilter.setStability(true, 30, 5000);
```

---

### 🎯 LongRangePrecision
For very long distances where percent filter helps.  
```cpp
tofFilter.setOffset(10);
tofFilter.setRangeLimits(15, 4000);
tofFilter.setPublishInterval(300);

tofFilter.setAlpha(true, 0.02f, 0.5f);
tofFilter.setDeadband(true, 5);
tofFilter.setDeltaNorm(true, 80);
tofFilter.setPercentFilter(true, 0.1f, 2500);
```

---

## License
MIT License

