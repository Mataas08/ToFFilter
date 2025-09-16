#include "ToFFilter.h"

// ------------------------------------------------------------
// Constructor
// Initializes internal variables (defaults are in header file).
// ------------------------------------------------------------
ToFFilter::ToFFilter() {}


// ------------------------------------------------------------
// Basic configuration setters
// ------------------------------------------------------------

// Set calibration offset (mm) → constant correction applied to all readings
void ToFFilter::setOffset(int mm) { OFFSET_MM = mm; }

// Define valid measurement range (mm). Values outside → 0.0 or NAN.
void ToFFilter::setRangeLimits(int minMm, int maxMm) {
  MIN_VALID_MM = minMm; 
  MAX_VALID_MM = maxMm;
}

// Set minimum interval (ms) between published results
void ToFFilter::setPublishInterval(unsigned long ms) { PUBLISH_INTERVAL = ms; }


// ------------------------------------------------------------
// Optional filter configuration
// ------------------------------------------------------------

// Enable/disable adaptive EMA smoothing and set alpha limits
void ToFFilter::setAlpha(bool enabled, float minAlpha, float maxAlpha) {
  ALPHA_ENABLED = enabled;
  ALPHA_MIN = minAlpha;
  ALPHA_MAX = maxAlpha;
}

// Enable/disable deadband filter (ignore changes smaller than mm)
void ToFFilter::setDeadband(bool enabled, int mm) {
  DEADBAND_ENABLED = enabled;
  DEADBAND_MM = mm;
}

// Enable/disable delta normalization (scales responsiveness)
void ToFFilter::setDeltaNorm(bool enabled, int mm) {
  DELTANORM_ENABLED = enabled;
  DELTA_NORM = mm;
}

// Enable/disable stability lock → freezes output if stable for too long
void ToFFilter::setStability(bool enabled, int mm, unsigned long ms) {
  STABILITY_LOCK = enabled;
  if (enabled) {
    STABILITY_THRESHOLD_MM = mm;
    STABILITY_TIME_MS = ms;
  }
}

// Enable/disable percent filter for large distances
// Ignores relative changes smaller than pct above given start distance
void ToFFilter::setPercentFilter(bool enabled, float pct, int startMm) {
  PERCENT_FILTER_ENABLED = enabled;
  PERCENT_THRESHOLD = pct;
  PERCENT_START_MM = startMm;
}


// ------------------------------------------------------------
// Main filter function
// Input: raw distance in mm
// Output: filtered distance in meters
// ------------------------------------------------------------
double ToFFilter::filter(int rawMm) {
  unsigned long now = millis();

  // Handle invalid input (0 or negative)
  if (rawMm <= 0) {
    return handleTimeout(now);
  }
  nanCount = 0;

  // Apply median filter (3 samples) to reduce spikes
  pushMedian(rawMm);
  int rawMedian = getMedian();

  // Apply offset correction
  int mm = rawMedian - OFFSET_MM;
  if (mm < 0) mm = 0;

  // Validate range
  if (mm < MIN_VALID_MM) return 0.0;
  if (mm > MAX_VALID_MM) return NAN;

  // Apply all active filters
  double stableValue = stabilizeCurve(mm);

  // Publish result at fixed interval
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublishedValue = stableValue / 1000.0; // convert mm → meters
    lastPublish = now;
    return lastPublishedValue;
  } else {
    return lastPublishedValue;
  }
}


// ------------------------------------------------------------
// Timeout handler
// Returns NAN after too many invalid samples in a row
// ------------------------------------------------------------
double ToFFilter::handleTimeout(unsigned long now) {
  nanCount++;
  if (nanCount >= NAN_THRESHOLD) {
    lastPublishedValue = NAN;
    lastPublish = now;
    return NAN;
  }
  return lastPublishedValue;
}


// ------------------------------------------------------------
// Main stabilization logic
// Applies stability lock, percent filter, deadband, and EMA
// ------------------------------------------------------------
double ToFFilter::stabilizeCurve(int mm) {
  // Initialize filter on first call
  if (filtered_mm < 0) {
    filtered_mm = mm;
    lastMovement = millis();
    return filtered_mm;
  }

  int diff = mm - (int)filtered_mm;
  int delta = abs(diff);

  // --- Stability lock ---
  if (STABILITY_LOCK) {
    if (delta > STABILITY_THRESHOLD_MM) {
      // Significant movement → reset freeze
      frozen = false;
      lastMovement = millis();
    } else {
      // No significant movement → check freeze time
      if (!frozen && (millis() - lastMovement > STABILITY_TIME_MS)) {
        frozen = true;
      }
    }
    if (frozen) {
      return filtered_mm; // hold last stable value
    }
  }

  // --- Percent filter (for large distances only) ---
  if (PERCENT_FILTER_ENABLED) {
    int start = (PERCENT_START_MM > 0) ? PERCENT_START_MM : (MAX_VALID_MM / 2);
    if (mm > start && filtered_mm > 0) {
      float percentChange = (float)abs(mm - (int)filtered_mm) / (float)filtered_mm;
      if (percentChange < PERCENT_THRESHOLD) {
        return filtered_mm;  // ignore small relative changes
      }
    }
  }

  // --- Deadband filter ---
  if (DEADBAND_ENABLED && delta <= DEADBAND_MM) {
    return filtered_mm;
  }

  // --- Adaptive EMA (Alpha filter) ---
  if (ALPHA_ENABLED) {
    // Compute logarithmic factor based on distance
    float logFactor = log10((float)mm + 10.0f) / log10((float)MAX_VALID_MM + 110.0f);

    // Compute delta factor (normalized change magnitude)
    float deltaFactor = 1.0f;
    if (DELTANORM_ENABLED) {
      deltaFactor = (float)delta / (float)DELTA_NORM;
      if (deltaFactor > 2.0f) deltaFactor = 2.0f;
    }

    // Compute sigmoid curve response
    float curve = 1.0f / (1.0f + expf(-(deltaFactor*1.5f - logFactor*3.5f)));

    // Blend alpha between min/max
    float alpha = ALPHA_MIN + (ALPHA_MAX - ALPHA_MIN) * curve;

    // Apply EMA update
    filtered_mm = filtered_mm + alpha * diff;
  } else {
    // EMA disabled → direct update (no smoothing)
    filtered_mm = mm;
  }

  return filtered_mm;
}


// ------------------------------------------------------------
// Median filter helpers
// ------------------------------------------------------------

// Push new sample into buffer (3 elements circular buffer)
void ToFFilter::pushMedian(int v) {
  medianBuf[medianIdx] = v;
  medianIdx = (medianIdx + 1) % 3;
  if (medianIdx == 0) medianFilled = true;
}

// Compute median of 3 samples (or fallback if not filled yet)
int ToFFilter::getMedian() {
  if (!medianFilled) return medianBuf[(medianIdx+2)%3];
  int a = medianBuf[0], b = medianBuf[1], c = medianBuf[2];
  if (a > b) swap(a,b);
  if (b > c) swap(b,c);
  if (a > b) swap(a,b);
  return b;
}

// Swap helper (for median computation)
void ToFFilter::swap(int &x, int &y) { int t=x; x=y; y=t; }
