#include <Arduino.h>
#include <HCSR04.h>
#include <LiquidCrystal_I2C.h>
#include "ViseurAutomatique.h"
#include "Alarm.h"
#include <U8g2lib.h>
#include <Wire.h>
#include <AccelStepper.h>


// --- Capteur de distance ---
#define TRIG_PIN 6
#define ECHO_PIN 7
HCSR04 hc(TRIG_PIN, ECHO_PIN);
float g_distance = 0;
int DISTANCE_ALARME = 15;

// --- Écran LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Viseur Automatique ---
ViseurAutomatique viseur(31 ,33 ,35 ,37, g_distance);

// --- Alarme ---
Alarm alarme(2, 3, 11, 4, &g_distance);

// --- MAX7219 (Matrice LED) ---
#define CLK_PIN 30
#define DIN_PIN 34
#define CS_PIN  32  // Chip Select

U8G2_MAX7219_8X8_F_4W_SW_SPI u8g2(
  U8G2_R0,       // rotation
  /* clock=*/ CLK_PIN, // pin Arduino reliée à CLK (horloge)
  /* data=*/ DIN_PIN,  // pin Arduino reliée à DIN (données)
  /* cs=*/ CS_PIN,    // pin Arduino reliée à CS (chip select)
  /* dc=*/ U8X8_PIN_NONE,
  /* reset=*/ U8X8_PIN_NONE
);

// --- Horloge ---
unsigned long _currentTime = 0;
unsigned long _lastMesureTime = 0;
const unsigned long INTERVALLE_MESURE = 50;

// Constantes pour l'écran LCD
const unsigned long LCD_DELAY_MS = 3000;

// Constantes pour le viseur automatique
const int ANGLE_MIN = 10;
const int ANGLE_MAX = 170;
const int PAS_PAR_TOUR = 2048;
const int DISTANCE_MIN_SUIVI = 20;  // en cm
const int DISTANCE_MAX_SUIVI = 60;  // en cm

// Constantes pour l'alarme
const int ALARME_COLOUR_A_R = 255;
const int ALARME_COLOUR_A_G = 0;
const int ALARME_COLOUR_A_B = 0;

const int ALARME_COLOUR_B_R = 255;
const int ALARME_COLOUR_B_G = 255;
const int ALARME_COLOUR_B_B = 0;

const unsigned long ALARME_TIMEOUT_MS = 3000;
const unsigned long VARIATION_TIMING_MS = 400;

// --- Gestion symbole non bloquante ---
String symboleActuel = "";
unsigned long symboleStartTime = 0;
const unsigned long SYMBOLE_DUREE = 3000;
bool symboleAffiche = false;


void setup() {
  Serial.begin(115200);

  // LCD
lcd.init();
lcd.backlight();
lcd.setCursor(0, 0);
lcd.print("2413645");
lcd.setCursor(0, 1);
lcd.print("Labo-07");
delay(LCD_DELAY_MS);
lcd.clear();

// Viseur
viseur.setAngleMin(ANGLE_MIN);
viseur.setAngleMax(ANGLE_MAX);
viseur.setPasParTour(PAS_PAR_TOUR);
viseur.setDistanceMinSuivi(DISTANCE_MIN_SUIVI);
viseur.setDistanceMaxSuivi(DISTANCE_MAX_SUIVI);
viseur.activer();

// Alarme
alarme.setColourA(ALARME_COLOUR_A_R, ALARME_COLOUR_A_G, ALARME_COLOUR_A_B);
alarme.setColourB(ALARME_COLOUR_B_R, ALARME_COLOUR_B_G, ALARME_COLOUR_B_B);
alarme.setDistance(DISTANCE_ALARME);
alarme.setTimeout(ALARME_TIMEOUT_MS);
alarme.setVariationTiming(VARIATION_TIMING_MS);


  u8g2.begin();
}

int LIMITE_INF = 30;
int LIMITE_SUP = 60;

void loop() {
  _currentTime = millis();

  // --- Mesure de distance ---
  if (_currentTime - _lastMesureTime >= INTERVALLE_MESURE) {
    g_distance = hc.dist();
    _lastMesureTime = _currentTime;
  }

  // --- Mise à jour des modules ---
  viseur.update(_currentTime);
  alarme.update(_currentTime, g_distance);

  // --- Affichage LCD ---
  afficherLCD(g_distance, (int)viseur.getAngle());

  // --- Commandes série ---
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "gDist") {
      // Afficher la distance
      Serial.print("gDist : ");
      Serial.println(g_distance);
      afficherSymbole("✔");
    } 
    else if (command.startsWith("cfg;alm;")) {
      int limit = command.substring(8).toInt();
      DISTANCE_ALARME = limit;
      alarme.setSeuil(limit);
      Serial.print("Alarme configurée à ");
      Serial.println(limit);
      afficherSymbole("✔");
    } 
    else if (command.startsWith("cfg;lim_inf;")) {
      int limit = command.substring(12).toInt();
      if (limit >= LIMITE_SUP) {
        Serial.println("Erreur – Limite inférieure plus grande que limite supérieure");
        afficherSymbole("🚫");
      } else {
        LIMITE_INF = limit;
        viseur.setLimites(LIMITE_INF, LIMITE_SUP);
        Serial.print("Limite inférieure configurée à ");
        Serial.println(limit);
        afficherSymbole("✔");
      }
    } 
    else if (command.startsWith("cfg;lim_sup;")) {
      int limit = command.substring(12).toInt();
      if (limit <= LIMITE_INF) {
        Serial.println("Erreur – Limite supérieure plus petite que limite inférieure");
        afficherSymbole("🚫");
      } else {
        LIMITE_SUP = limit;
        viseur.setLimites(LIMITE_INF, LIMITE_SUP);
        Serial.print("Limite supérieure configurée à ");
        Serial.println(limit);
        afficherSymbole("✔");
      }
    } 
    else {
      // Commande inconnue
      Serial.println("Commande inconnue.");
      afficherSymbole("X");
    }
  }
}

void afficherLCD(int distance, int angle) {
  lcd.setCursor(0, 0);
  lcd.print("Dist : ");
  lcd.print(distance);
  lcd.print(" cm    ");

  lcd.setCursor(0, 1);
  if (distance < 15) {  // Si la distance est inférieure à 15 cm
    lcd.print("Obj : Trop pres");
  } else if (distance > 60) {
    lcd.print("Obj : Trop loin");
  } else {
    lcd.print("Obj : ");
    lcd.print(angle);
    lcd.print(" deg     ");
  }
}

void afficherSymbole(const String& symbole) {
  symboleActuel = symbole;
  symboleStartTime = _currentTime;
  symboleAffiche = false;

  u8g2.clearBuffer();

  if (symboleActuel == "✔") {
    // Coche penchée vers la droite
    u8g2.drawPixel(0, 4);
    u8g2.drawPixel(1, 5);
    u8g2.drawPixel(2, 6);
    u8g2.drawPixel(3, 5);
    u8g2.drawPixel(4, 4);
    u8g2.drawPixel(5, 3);
    u8g2.drawPixel(6, 2);
  } 
  else if (symboleActuel == "X") {
    for (int i = 0; i < 8; i++) {
      u8g2.drawPixel(i, i);
      u8g2.drawPixel(7 - i, i);
    }
  } 
  else if (symboleActuel == "🚫") {
    u8g2.drawCircle(3, 3, 3);       // petit cercle
    u8g2.drawLine(0, 0, 6, 6);      // ligne diagonale
  }

  u8g2.sendBuffer(); // Affiche
  delay(3000);
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}