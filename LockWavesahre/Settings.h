#include <Preferences.h>

Preferences preferences;

String serverName = "";
String nodeId = "";
String clientAlias = "";

String displayIP = "";
String gatewayIP = "";
String subnetMask = "";
String dnsIP = "";
uint8_t unlockTime = 20;

void loadSettings() {
  serverName = preferences.getString("serverName", "");
  nodeId = preferences.getString("nodeId", "node_tabule");
  clientAlias = preferences.getString("clientAlias", "Modul zamku");

  displayIP = preferences.getString("displayIP", "");
  gatewayIP = preferences.getString("gatewayIP", "");
  subnetMask = preferences.getString("subnetMask", "");
  dnsIP = preferences.getString("dnsIP", "");
  unlockTime = preferences.getUChar("unlockTime", 20);
}

void forceSystemReset() {
  Serial.print("Restarting...");
  delay(500);
	ESP.restart();
}

void storeServerSettings() {
  Serial.printf("Store nodeId: ", nodeId);

  preferences.putString("serverName", serverName);
  preferences.putString("nodeId", nodeId);

  preferences.putString("displayIP", displayIP);
  preferences.putString("gatewayIP", gatewayIP);
  preferences.putString("subnetMask", subnetMask);
  preferences.putString("dnsIP", dnsIP);
  preferences.putUChar("unlockTime", unlockTime);
  
}

void storeDisplaySettings() {
  preferences.putString("clientAlias",clientAlias);
}

void setupSettings() {
  printf("Setup settings\n");
  preferences.begin("displayPref", false);
  loadSettings();
}