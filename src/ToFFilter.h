#pragma once
#include <Arduino.h>
#include <math.h>

class ToFFilter {
 public:
  ToFFilter();

  // --- Basic configuration ---
  void setOffset(int mm);                        // Offset correction in mm
  void setRangeLimits(int minMm, int maxMm);     // Valid measurement range
  void setPublishInterval(unsigned long ms);     // Min interval between outputs

  // --- Alpha (EMA) filter ---
  void setAlpha(bool enabled, float minAlpha = 0.02f, float maxAlpha = 0.6f); // Adaptive smoothing

  // --- Deadband filter ---
  void setDeadband(bool enabled, int mm = 2);    // Ignore small changes

  // --- DeltaNorm ---
  void setDeltaNorm(bool enabled, int mm = 60);  // Normalize reactivity

  // --- Stability lock ---
  void setStability(bool enabled, int mm = 10, unsigned long ms = 2000); // Freeze when stable

  // --- Percent filter ---
  void setPercentFilter(bool enabled, float pct = 0.05f, int startMm = -1); // Relative filter at long range

  // --- Main filter ---
  double filter(int rawMm);  // Input mm → Output meters (or NAN/0.0)

 private:
  // General parameters
  int OFFSET_MM = 10;
  int MIN_VALID_MM = 15;
  int MAX_VALID_MM = 2000;
  unsigned long PUBLISH_INTERVAL = 200;
  int NAN_THRESHOLD = 2;

  // Alpha filter
  bool ALPHA_ENABLED = false;
  float ALPHA_MIN = 0.02f;
  float ALPHA_MAX = 0.6f;

  // Deadband
  bool DEADBAND_ENABLED = false;
  int DEADBAND_MM = 2;

  // DeltaNorm
  bool DELTANORM_ENABLED = false;
  int DELTA_NORM = 60;

  // Stability lock
  bool STABILITY_LOCK = false;
  int STABILITY_THRESHOLD_MM = 10;
  unsigned long STABILITY_TIME_MS = 2000;
  unsigned long lastMovement = 0;
  bool frozen = false;

  // Percent filter
  bool PERCENT_FILTER_ENABLED = false;
  float PERCENT_THRESHOLD = 0.05f;
  int PERCENT_START_MM = -1;

  // Internal state
  int nanCount = 0;
  float filtered_mm = -1;
  double lastPublishedValue = NAN;
  unsigned long lastPublish = 0;

  // Median filter
  int medianBuf[3] = {0,0,0};
  int medianIdx = 0;
  bool medianFilled = false;

  // Helpers
  double handleTimeout(unsigned long now);
  double stabilizeCurve(int mm);
  void pushMedian(int v);
  int getMedian();
  void swap(int &x, int &y);
};
