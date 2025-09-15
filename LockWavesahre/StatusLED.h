#include "WS_GPIO.h"

void setupLED() {
   
}

bool lastLanConnected = true;
bool lastStatusServerConnected = true;

void loopLED() {
  if (!lanConnected) {
    statusServerConnected = false;
  }
  if (lastLanConnected != lanConnected) {
    lastLanConnected = lanConnected;
    Serial.print("lanConnected = ");
    Serial.println(lanConnected);
  }
  if (lastStatusServerConnected != statusServerConnected) {
    lastStatusServerConnected = statusServerConnected;
    Serial.print("statusServerConnected = ");
    Serial.println(statusServerConnected);
  }

  if (!lanConnected && !statusServerConnected) {
    RGB_Light(64, 0, 0);
  } else
  if (statusServerConnected) {
    RGB_Light(0, 64, 0);
  } else {
    RGB_Light(0, 0, 64);
  }
   
}