#pragma once

#include <Arduino.h>

enum AlarmState {
  OFF,
  WATCHING,
  ON,
  TESTING
};

class Alarm {
public:
  Alarm(int rPin, int gPin, int bPin, int buzzerPin, float* distancePtr);
  void update(unsigned long currentTime, float distance);
  void setColourA(int r, int g, int b);
  void setColourB(int r, int g, int b);
  void setVariationTiming(unsigned long ms);
  void setDistance(float d);
  void setTimeout(unsigned long ms);
  void turnOff();
  void turnOn();
  void test();
  void setSeuil(int seuil);
  AlarmState getState() const;

private:
  int seuil;  // Seuil de déclenchement de l'alarme
  int _rPin, _gPin, _bPin, _buzzerPin;
  unsigned long _currentTime = 0;
  int _colA[3], _colB[3];
  bool _currentColor = false;
  float* _distance;
  float _distanceTrigger = 10.0;
  unsigned long _variationRate = 500;
  unsigned long _timeoutDelay = 3000;
  unsigned long _lastUpdate = 0;
  unsigned long _lastDetectedTime = 0;
  unsigned long _testStartTime = 0;
  AlarmState _state = OFF;
  bool _turnOnFlag = false;
  bool _turnOffFlag = false;

  void _setRGB(int r, int g, int b);
  void _turnOff();
  void _offState();
  void _watchState();
  void _onState();
  void _testingState();
};
