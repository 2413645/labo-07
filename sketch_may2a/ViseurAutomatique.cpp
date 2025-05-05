#include "ViseurAutomatique.h"

ViseurAutomatique::ViseurAutomatique(int p1, int p2, int p3, int p4, float& distanceRef)
  : _stepper(AccelStepper::FULL4WIRE, p1, p3, p2, p4), _distance(distanceRef) {
  _stepper.setMaxSpeed(500);
  _stepper.setAcceleration(300);
}

void ViseurAutomatique::update(unsigned long currentTime) {
  switch (_etat) {
    case INACTIF: _inactifState(currentTime); break;
    case SUIVI: _suiviState(currentTime); break;
    case REPOS: _reposState(currentTime); break;
  }
  _stepper.run();
}

void ViseurAutomatique::setAngleMin(float angle) { _angleMin = angle; }
void ViseurAutomatique::setAngleMax(float angle) { _angleMax = angle; }
void ViseurAutomatique::setPasParTour(int steps) { _stepsPerRev = steps; }
void ViseurAutomatique::setDistanceMinSuivi(float distanceMin) { _distanceMinSuivi = distanceMin; }
void ViseurAutomatique::setDistanceMaxSuivi(float distanceMax) { _distanceMaxSuivi = distanceMax; }
void ViseurAutomatique::activer() { _etat = REPOS; }
void ViseurAutomatique::desactiver() { _etat = INACTIF; }

float ViseurAutomatique::getAngle() const {
  return _stepper.currentPosition() * 360.0 / _stepsPerRev;
}

int ViseurAutomatique::getLimiteSup() const { return _limiteSup; }
int ViseurAutomatique::getLimiteInf() const { return _limiteInf; }

void ViseurAutomatique::setLimites(int inf, int sup) {
  if (inf >= 10 && sup <= 170 && inf < sup) {
    _limiteInf = inf;
    _limiteSup = sup;
  }
}

long ViseurAutomatique:: _angleEnSteps(float angle) const {
  return angle * _stepsPerRev / 360.0;
}

void ViseurAutomatique::_inactifState(unsigned long cT) {
  // Ne rien faire
}

void ViseurAutomatique::_reposState(unsigned long cT) {
  if (_distance >= _distanceMinSuivi && _distance <= _distanceMaxSuivi) {
    _etat = SUIVI;
  }
}

void ViseurAutomatique::_suiviState(unsigned long cT) {
  if (_distance < _distanceMinSuivi || _distance > _distanceMaxSuivi) {
    _etat = REPOS;
    return;
  }

  // Calculer un angle proportionnel à la distance
  float range = _distanceMaxSuivi - _distanceMinSuivi;
  float ratio = (_distance - _distanceMinSuivi) / range;
  float angle = _angleMin + (_angleMax - _angleMin) * ratio;

  // Contrainte dans les limites
  if (angle < _limiteInf) angle = _limiteInf;
  if (angle > _limiteSup) angle = _limiteSup;

  long targetSteps = _angleEnSteps(angle);
  _stepper.moveTo(targetSteps);
}
