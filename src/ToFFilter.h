#pragma once
#include <Arduino.h>
#include <math.h>

class ToFFilter {
 public:
  ToFFilter();

  // --- konfiguracja (ustawiana w setup()) ---
  void setOffset(int mm);                  // offset w mm
  void setRangeLimits(int minMm, int maxMm); // zakres ważnych wartości (np. <15=0, >2000=NAN)
  void setPublishInterval(unsigned long ms);// min. odstęp publikacji
  void setAlphaLimits(float minAlpha, float maxAlpha); // zakres współczynnika filtra
  void setDeadband(int mm);                // strefa martwa w mm
  void setDeltaNorm(int mm);               // normalizacja deltaFactor (np. 60 mm)
  
  // --- główna funkcja filtra ---
  double filter(int rawMm); // przyjmuje mm z czujnika, zwraca przefiltrowane w metrach

 private:
  // parametry
  int OFFSET_MM = 10;
  int MIN_VALID_MM = 15;
  int MAX_VALID_MM = 2000;
  unsigned long PUBLISH_INTERVAL = 200;
  int NAN_THRESHOLD = 2;

  float ALPHA_MIN = 0.02f;
  float ALPHA_MAX = 0.6f;
  int DEADBAND_MM = 2;
  int DELTA_NORM = 60;

  // stan wewnętrzny
  int nanCount = 0;
  float filtered_mm = -1;
  double lastPublishedValue = NAN;
  unsigned long lastPublish = 0;

  // median filter
  int medianBuf[3] = {0,0,0};
  int medianIdx = 0;
  bool medianFilled = false;

  // funkcje pomocnicze
  double handleTimeout(unsigned long now);
  double stabilizeCurve(int mm);
  void pushMedian(int v);
  int getMedian();
  void swap(int &x, int &y);
};
