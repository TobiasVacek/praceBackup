#include <WebSocketsClient.h>
#include <ArduinoJson.h>
//#include <Settings.h>

#define DEBUG_WEBSOCKETS(...) printf( __VA_ARGS__ )

WebSocketsClient webSocket;
unsigned long messageId = 0;
String lastServerName = "";

String getClientId() {
  return getChipIdString() + "_zamky";
}

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		printf("%02X ", *src);
		src++;
	}
  printf("\n");
}

void accept(String clientId, String messageId) {
  webSocket.sendTXT("{'action':'ACCEPT','clientId':'" + clientId + "','nodeId':'" + nodeId + "','messageId':'" + messageId + "','data':{}}");
}

int getMessageId() {
  messageId++;
  return messageId;
}

void parseMessage(uint8_t* payload) {
  printf("[WSc] get text: %s\n", payload);

  DynamicJsonDocument doc(500);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) { 
    printf("deserializeJson() failed: ");
    printf("%s\r\n", error.f_str());
    return;
  } else {
    String cliId = doc["clientId"];
    String action = doc["action"];
    String messageId = doc["messageId"]; 
    String type = doc["data"]["type"]; 
    bool sendAcceptation = true; 
    if (action.equals("MESSAGE")) {
      //Serial.printf("[WSc] message targetClientId = %s\n", doc["data"]["targetClientId"]);
      if (doc["data"]["targetClientId"] == getClientId()) {
        printf("[WSc] processing action: ");
        printf("%s", action);
        printf(" - ");
        printf("%s\r\n", type);

        if (type.equals("askForAlias")) { 
          printf("Sending alias JSON\r\n");
          webSocket.sendTXT("{'action':'MESSAGE','clientId':'','nodeId':'" + nodeId + "','messageId':'" + getMessageId() + "','data':{'type':'clientAlias','clientAlias':'" + clientAlias + "'}}");
        } 
        else
        if (type.equals("setClientAlias")) {
          printf("[WSc] store settings\r\n");  
          String newClientAlias = doc["data"]["clientAlias"]; ;
          if (newClientAlias != nullptr && newClientAlias != "") {
            clientAlias = newClientAlias;
            printf("[WSc] newClientAlias = ");
            printf("%s\r\n", clientAlias);
            storeDisplaySettings();
          }
        } else
        if (sendAcceptation) {
          accept(cliId, messageId);
        }
        return;
      }      

      byte keyDoorId = 0;
      byte keyDoorLedId = 0;
      byte keyModule = 0;
      byte timeSeconds = unlockTime;
      if (type.equals("invoke")) {
        printf("[WSc] processing invoke action: %s - %s\n", action, type);
        keyDoorId = doc["data"]["patient"]["keyDoorId"];
        keyDoorLedId = doc["data"]["patient"]["keyDoorLedId"];
        keyModule = doc["data"]["patient"]["keyModule"];
      }

      if (type.equals("unlock")) {
        printf("[WSc] processing unlock action: %s - %s\n", action, type);
        keyDoorId = doc["data"]["keyDoorId"];
        keyDoorLedId = doc["data"]["keyDoorLedId"];
        keyModule = doc["data"]["keyModule"];
      }

      //  -----   LOCK RELAY  ---------
      if (keyDoorId > 0 && keyModule > 0) {
        printf("[WSc] processing invoke unlock: %d for %d seconds\n", keyDoorId, timeSeconds); 
        
        //unlock door
        unlockDoor(keyDoorId, timeSeconds);

        if (sendAcceptation) {
          accept(cliId, messageId);
        }
      } else 
      if (keyDoorId > 0 && keyModule == 0) {
        // unlock master relay
        printf("[WSc] processing invoke unlock local relay: %d for %d seconds\n", keyDoorId, timeSeconds); 

        unlockLocalDoor(keyDoorId, timeSeconds);

        if (sendAcceptation) {
          accept(cliId, messageId);
        } 
      }
      
      //  -----   LED RELAY ---------
      if (keyDoorLedId > 0 && keyModule > 0) {
        printf("[WSc] processing invoke unlock LED: %d for %d seconds\n", keyDoorLedId, timeSeconds); 
        //light LED
        unlockDoor(keyDoorLedId, timeSeconds);
      } else 
      if (keyDoorLedId > 0 && keyModule == 0) {
        // switch on master relay
        printf("[WSc] processing invoke unlock local LED relay: %d for %d seconds\n", keyDoorLedId, timeSeconds); 
        unlockLocalDoor(keyDoorLedId, timeSeconds);
      }
    }    
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
		case WStype_DISCONNECTED:
      if (serverName == "") {
        setSystemMessage("Nakonfigurujte zamky na adrese: " + ETH.localIP().toString());
      } else {
        printf("[WSc] Pripojuji na: %s, IP displeje: %s\n", serverName, ETH.localIP().toString());
        setSystemMessage("Pripojuji k:" + serverName + ",IP:" + ETH.localIP().toString());
      }
      statusServerConnected = false;
      break;
		case WStype_CONNECTED:
			printf("[WSc] Connected to url: %s\n", payload);
      if (lastServerName == "") {
        setSystemMessage("Nakonfigurujte zamky na adrese: " + ETH.localIP().toString());
      } else {
        setSystemMessage("");
      }
			// send message to server when Connected
			printf("[WSc] Connecting to node: %s\n", nodeId);
			webSocket.sendTXT("{'action':'CONNECT','clientId':'" + getClientId() + "','nodeId':'" + nodeId + "','messageId':'" + getMessageId() + "','data':{'clientName':'zamky','clientAlias':'" + clientAlias + "'}}");
		  lanConnected = true;
      statusServerConnected = true;
    	break;
		case WStype_TEXT:
      parseMessage(payload);
			break;
		case WStype_BIN:
			printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);
			break;
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
	}
}

void setupBoomerang() {
  if (serverName != "") {
    lastServerName = serverName;
    printf("[WSc] connecting boomerang at: %s\n", lastServerName);  
    webSocket.begin(lastServerName, 8282, "/ws/"); 
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
    webSocket.enableHeartbeat(15000, 5000, 1);
  }
}

void loopBoomerang() {
  if (lastServerName != serverName) {
    setupBoomerang();
  }
  if (!((ETH.localIP().toString() == "") || (ETH.localIP().toString() == "0.0.0.0") || !lanConnected)) {
    webSocket.loop();
  }
}