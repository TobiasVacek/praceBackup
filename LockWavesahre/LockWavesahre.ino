#include "WS_Serial.h"

#include "ChipId.h"
#include "Settings.h"
#include "NetworkState.h"
#include "Relays.h"
#include "WS_ETH.h"
#include "Lan.h"
#include "StatusLED.h";
#include "WifiAP.h"
#include "WebServer.h"
#include "Boomerang.h"
// #include "Sse.h"

uint32_t Simulated_time=0;

void setup() {
  //Serial.begin(115200);
  delay(1000);

  setupSettings();
  setupRelay();
  setupLED();
  setupLAN();

  setupWifi();
  Serial_Init();
  printf("Vyvolavak lock module just started\n");

  delay(1000);
  setupServer();
  delay(1000);
  //setupSse();
}

void loop() {
  if ((ETH.localIP().toString() == "") || (ETH.localIP().toString() == "0.0.0.0") || !lanConnected) {
    setSystemMessage("Pripojte modul k LAN (" + getChipIdString() + "), IP=" + ETH.localIP().toString());
  } else {
    if (serverName == "") {
      setSystemMessage("Zadejte nazev serveru na strance http://" + ETH.localIP().toString());
    }
  }

  wifiEnabled = !lanConnected;

  loopServer();
  loopBoomerang();
  loopRelay();
  loopLED();

  wifiEnabled = !lanConnected;
  loopWifi();
  //loopSse();
}
