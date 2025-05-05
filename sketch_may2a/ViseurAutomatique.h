#pragma once
#include <AccelStepper.h>
#include <Arduino.h>

enum EtatViseur { INACTIF, SUIVI, REPOS };

class ViseurAutomatique {
public:
  ViseurAutomatique(int p1, int p2, int p3, int p4, float& distanceRef);
  void update(unsigned long currentTime);
  void setAngleMin(float angle);
  void setAngleMax(float angle);
  void setPasParTour(int steps);
  void setDistanceMinSuivi(float distanceMin);
  void setDistanceMaxSuivi(float distanceMax);
  float getAngle() const;
  void activer();
  void desactiver();
  int getLimiteSup() const;
  int getLimiteInf() const;
  void setLimites(int inf, int sup);

private:
  AccelStepper _stepper;
  float& _distance;
  float _angleMin = 0.0;
  float _angleMax = 180.0;
  int _stepsPerRev = 2048;
  float _distanceMinSuivi = 30.0;
  float _distanceMaxSuivi = 60.0;
  int _limiteInf = 10;
  int _limiteSup = 170;
  EtatViseur _etat = INACTIF;

  void _inactifState(unsigned long cT);
  void _suiviState(unsigned long cT);
  void _reposState(unsigned long cT);
  long _angleEnSteps(float angle) const;
};
