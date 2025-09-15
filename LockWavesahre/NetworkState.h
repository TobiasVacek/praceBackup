#pragma once

bool wifiStarted = false;

String prefixText = "";
String displayText = "";
String systemMessage = "";

void setDisplayText(String txt) {
  prefixText = "";
  displayText = txt;
  if (txt != "") {
    printf("Display text: %s\n", txt);
  }
}

void setSystemMessage(String txt) {
  if (txt != systemMessage) {
    systemMessage = txt;
    if (txt != "") {
      printf("System message: %s\n", txt);
    }
  }
}

void networkConnected(String net, String ip) {
  setSystemMessage("");
}

void networkDisconnected() {
  setSystemMessage("Pripojte displej k LAN");
}