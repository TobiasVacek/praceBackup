
#pragma once

#include <ETH.h>
#include <SPI.h>
#include <ESPmDNS.h>

//#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
//#define ETH_PHY_POWER 12

#define ETH_PHY_TYPE ETH_PHY_W5500
#define ETH_PHY_ADDR 1
#define ETH_PHY_CS   16
#define ETH_PHY_IRQ  12
#define ETH_PHY_RST  39

// SPI pins
#define ETH_SPI_SCK  15
#define ETH_SPI_MISO 14
#define ETH_SPI_MOSI 13

#define HOSTNAME     "vyvolavak-lock"

static bool lanConnected = false;
bool statusServerConnected = false;
IPAddress localIPAddress;
String localIP = "";
String macAddress = "";

bool staticIp = false;
IPAddress _displayIP;
IPAddress _gatewayIP;
IPAddress _subnetMask;
IPAddress _dnsIP;

// void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
//   switch (event) {
//     case ARDUINO_EVENT_ETH_START:
//       printf("ETH Started\r\n");
//       //set eth hostname here
//       ETH.setHostname("esp32-eth0");
//       break;
//     case ARDUINO_EVENT_ETH_CONNECTED: printf("ETH Connected\r\n"); break;
//     case ARDUINO_EVENT_ETH_GOT_IP:    printf("ETH Got IP: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif)); //printf("%s\r\n",ETH);
//       ETH_ip = ETH.localIP(); 
//       printf("ETH Got IP: %d.%d.%d.%d\n", ETH_ip[0], ETH_ip[1], ETH_ip[2], ETH_ip[3]);
// #if USE_TWO_ETH_PORTS
//       // printf("%d\r\n",ETH1);
// #endif
//       eth_connected = true;
//       break;
//     case ARDUINO_EVENT_ETH_LOST_IP:
//       printf("ETH Lost IP\r\n");
//       eth_connected = false;
//       break;
//     case ARDUINO_EVENT_ETH_DISCONNECTED:
//       printf("ETH Disconnected\r\n");
//       eth_connected = false;
//       break;
//     case ARDUINO_EVENT_ETH_STOP:
//       printf("ETH Stopped\r\n");
//       eth_connected = false;
//       break;
//     default: break;
//   }
// }

void setupMDNS() {
  printf("Initialize mDNS\n");

  // Initialize mDNS
  esp_err_t err = mdns_init();
  if (err) {
      printf("MDNS Init failed: %d\n", err);
      return;
  }

  //set hostname
  mdns_hostname_set(HOSTNAME); 
  mdns_instance_name_set(HOSTNAME); 
  mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
  
  /*if (!MDNS.begin("vyvolavak-lock")) {   // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");
    // while(1) {
    //   delay(1000);
    // }
  } else {
    Serial.println("mDNS responder started");
  }
  */
}

void onLanEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      printf("ETH Started\n");
      //set eth hostname here
      ETH.setHostname(HOSTNAME);
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      lanConnected = true;
      printf("ETH Connected\n");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
    case ARDUINO_EVENT_ETH_GOT_IP6:
      lanConnected = true;
  
      setupMDNS();

      localIPAddress = ETH.localIP(); 
      printf("ETH IP: %d.%d.%d.%d\n", localIPAddress[0], localIPAddress[1], localIPAddress[2], localIPAddress[3]);
      localIP = localIPAddress.toString();
      macAddress = ETH.macAddress();
      printf("ETH MAC: ");
      printf("%s", macAddress.c_str());
      printf(", IPv4: ");
      printf("%s\n", localIP.c_str());

      networkConnected("LAN", localIP);
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      lanConnected = false;
      statusServerConnected = false;
      networkDisconnected();
      printf("ETH Disconnected\n");
      break;
    case ARDUINO_EVENT_ETH_STOP:
      lanConnected = false;
      statusServerConnected = false;
      networkDisconnected();
      printf("ETH Stopped\n");
      break;
    default:
      break;
  }
}

void updateLANSettings() {
  printf("Update LAN settings\r\n");
  staticIp = false;
  if (displayIP != "" && _displayIP.fromString(displayIP)) {
    staticIp = true;
    if (!_gatewayIP.fromString(gatewayIP) 
        || !_subnetMask.fromString(subnetMask)
        || !_dnsIP.fromString(dnsIP)
        ) {
      staticIp = false;
    }
  } 

 if (staticIp) {
    printf("Set static IP\r\n");
    ETH.config(_displayIP, _gatewayIP, _subnetMask, _dnsIP, _dnsIP);
  } else {
    printf("DHCP will be used\n");
  }

}

void setupLAN() {
  printf("LAN Start\r\n");
  RTC_Init();
  // ETH_Init();
  ETH_Init(onLanEvent);
  updateLANSettings();
}
