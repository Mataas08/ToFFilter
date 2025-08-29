#include "ToFFilter.h"

ToFFilter::ToFFilter() {}

void ToFFilter::setOffset(int mm) { OFFSET_MM = mm; }
void ToFFilter::setRangeLimits(int minMm, int maxMm) {
  MIN_VALID_MM = minMm; 
  MAX_VALID_MM = maxMm;
}
void ToFFilter::setPublishInterval(unsigned long ms) { PUBLISH_INTERVAL = ms; }
void ToFFilter::setAlphaLimits(float minAlpha, float maxAlpha) {
  ALPHA_MIN = minAlpha; 
  ALPHA_MAX = maxAlpha;
}
void ToFFilter::setDeadband(int mm) { DEADBAND_MM = mm; }
void ToFFilter::setDeltaNorm(int mm) { DELTA_NORM = mm; }

double ToFFilter::filter(int rawMm) {
  unsigned long now = millis();

  if (rawMm <= 0) {
    return handleTimeout(now);
  }
  nanCount = 0;

  // filtr medianowy
  pushMedian(rawMm);
  int rawMedian = getMedian();

  // offset
  int mm = rawMedian - OFFSET_MM;
  if (mm < 0) mm = 0;

  // zakres
  if (mm < MIN_VALID_MM) return 0.0;
  if (mm > MAX_VALID_MM) return NAN;

  // stabilizacja
  double stableValue = stabilizeCurve(mm);

  // publikacja co PUBLISH_INTERVAL
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublishedValue = stableValue / 1000.0;
    lastPublish = now;
    return lastPublishedValue;
  } else {
    return lastPublishedValue;
  }
}

double ToFFilter::handleTimeout(unsigned long now) {
  nanCount++;
  if (nanCount >= NAN_THRESHOLD) {
    lastPublishedValue = NAN;
    lastPublish = now;
    return NAN;
  }
  return lastPublishedValue;
}

double ToFFilter::stabilizeCurve(int mm) {
  if (filtered_mm < 0) {
    filtered_mm = mm;
    return filtered_mm;
  }

  int diff = mm - (int)filtered_mm;
  int delta = abs(diff);

  // deadband
  if (delta <= DEADBAND_MM) {
    return filtered_mm;
  }

  // logarytmiczne dopasowanie
  float logFactor = log10((float)mm + 10.0f) / log10((float)MAX_VALID_MM + 10.0f);

  // delta factor
  float deltaFactor = (float)delta / (float)DELTA_NORM;
  if (deltaFactor > 2.0f) deltaFactor = 2.0f;

  // sigmoida
  float curve = 1.0f / (1.0f + expf(-(deltaFactor*1.5f - logFactor*3.5f)));

  // α
  float alpha = ALPHA_MIN + (ALPHA_MAX - ALPHA_MIN) * curve;

  // EMA
  filtered_mm = filtered_mm + alpha * diff;

  return filtered_mm;
}

void ToFFilter::pushMedian(int v) {
  medianBuf[medianIdx] = v;
  medianIdx = (medianIdx + 1) % 3;
  if (medianIdx == 0) medianFilled = true;
}

int ToFFilter::getMedian() {
  if (!medianFilled) return medianBuf[(medianIdx+2)%3];
  int a = medianBuf[0], b = medianBuf[1], c = medianBuf[2];
  if (a > b) swap(a,b);
  if (b > c) swap(b,c);
  if (a > b) swap(a,b);
  return b;
}

void ToFFilter::swap(int &x, int &y) { int t=x; x=y; y=t; }
