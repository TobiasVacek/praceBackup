#include <WiFi.h>

const char* ssid = "vyvolavak_lock_";
const char* wifiPassword = "ehicehic";

String wifiIP = "";
bool wifiEnabled = false;
IPAddress apIP(192, 168, 4, 1);

void checkWifiStarted() {
  if (wifiEnabled && !wifiStarted) {
    Serial.println("\nStarting wifi");
    WiFi.softAPdisconnect(false);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid + getChipIdString(), wifiPassword);
    //delay(100);
    //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    wifiIP = WiFi.softAPIP().toString();    
    wifiStarted = true;

    printf("\nWiFi AP started\n");
    printf("Local WiFi IP: ");
    printf("%s\n", WiFi.softAPIP().toString());
  }
}

void checkWifiStopped() {
  if (!wifiEnabled && wifiStarted) {
    WiFi.softAPdisconnect(true);
    wifiIP = "";
    wifiStarted = false;
    printf("\nWiFi AP stopped\n");
  }
}

void setupWifi() {
  printf("Setup WiFi\n");
}

void loopWifi() {
  if (wifiEnabled) {
    checkWifiStarted();
  } else {
    checkWifiStopped();
  }
}