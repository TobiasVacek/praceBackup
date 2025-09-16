#include <HTTPClient.h>
#include <ArduinoJson.h>
//#include <Settings.h>

String lastSseServer = "";

HTTPClient httpClient;
WiFiClient* stream = NULL;
const String terminatingChar = "\n\n\r\n";
const byte pingIntervalSeconds = 17;  // 15s per ping + 2s for possible delay during calculation
boolean sseConnected = false;

String returnSubstring(String message, String fromString, String toString, boolean includeEdges = false, boolean findLast = false) {

  //returns substring of message between two given Strings

  int startIndex = message.indexOf(fromString);
  int endIndex = findLast ? message.lastIndexOf(toString) : message.indexOf(toString, startIndex + fromString.length());

  if (startIndex == -1 || endIndex == -1) {
    return "";
  }
  if (includeEdges) {
    return message.substring(startIndex, endIndex + toString.length());
  }
  return message.substring(startIndex + fromString.length(), endIndex);
}

void parseSseMessage(String type, String payload) {
  printf("[Sse] get text (%s): %s\n", type.c_str(), payload.c_str());

  DynamicJsonDocument doc(500);
  DeserializationError error = deserializeJson(doc, payload.c_str());
  if (error) {
    printf("deserializeJson() failed: ");
    printf("%s\n", error.f_str());
    return;
  }

  byte timeSeconds = unlockTime;
  byte keyModuleId;
  byte keyDoorId;
  byte keyDoorLedId;

  if (type.equals("unlock")) {
    keyModuleId = doc["keyModuleId"];
    keyDoorId = doc["keyDoorId"];
    keyDoorLedId = doc["keyDoorLedId"];

    if (keyDoorId > 0 && keyModuleId == 0) {
      // unlock master relay
      printf("[WSc] processing invoke unlock local relay: %d for %d seconds\n", keyDoorId, timeSeconds);
      unlockLocalDoor(keyDoorId, timeSeconds);
    }

    //  -----   LED RELAY ---------
    if (keyDoorLedId > 0 && keyModuleId == 0) {
      // switch on master relay
      printf("[WSc] processing invoke unlock local LED relay: %d for %d seconds\n", keyDoorLedId, timeSeconds);
      unlockLocalDoor(keyDoorLedId, timeSeconds);
    }
  }
}

void parseRawMessage(uint8_t buff[512], int size) {

  String message = (char*)buff;
  static String partialMessage = "";
  boolean recievedWholeMessage = true;

  String event = "";
  String data = "";
  int id = -1;
  while (recievedWholeMessage) {
    message = partialMessage + message;
    int endIndex = message.indexOf(terminatingChar);

    if (endIndex != -1) {
      partialMessage = message.substring(endIndex + terminatingChar.length());
      message = message.substring(0, endIndex + terminatingChar.length());

      if (message.indexOf("id:") != -1) {
        id = returnSubstring(message, "id:", "\n").toInt();
      }

      if (message.indexOf("event:") != -1) {
        event = returnSubstring(message, "event:", "\n");
      }

      if (message.indexOf("data:") != -1) {
        data = returnSubstring(message, "{", "}", true, true);
      }
      message = "";
      parseSseMessage(event, data);

    } else {
      partialMessage = message;
      message = "";
      recievedWholeMessage = false;
    }
  }
}


boolean checkConnected() {

  String url = serverName + "/api/sse/listen";

  boolean justConnected = false;
  if (!httpClient.connected() || !sseConnected) {
    printf("[Sse] connecting SSE at: ");
    printf("%s\n", url.c_str());
    statusServerConnected = false;
    stream = NULL;
    httpClient.begin(url);
    justConnected = true;
  }

  if (httpClient.connected() || justConnected || !sseConnected) {
    if (stream == NULL) {
      printf("[Sse] connected - get stream\n");
      int httpCode = httpClient.GET();
      printf("[Sse] SSE response code: %d\n", httpCode);

      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled

        // file found at server
        if (httpCode == HTTP_CODE_OK) {

          // get length of document (is -1 when Server sends no Content-Length header)
          int len = httpClient.getSize();
          sseConnected = true;
          // create buffer for read
          uint8_t buff[128] = { 0 };

          // get tcp stream
          stream = httpClient.getStreamPtr();
          statusServerConnected = true;
          return true;
        } else {
          stream = NULL;
        }
      } else {
        stream = NULL;
      }
    }
  } else {
    printf("[Sse] NOT connected");
    stream = NULL;
  }

  return stream != NULL;
}

void setupSse() {
  if (serverName != "") {
    // server name changed, close connection
    if (lastSseServer != serverName && httpClient.connected()) {
      httpClient.end();
      statusServerConnected = false;
    }

    lastSseServer = serverName;
  }
}

void loopSse() {
  if (lastSseServer != serverName) {
    setupSse();
  }

  if (checkConnected() && stream != NULL) {
    // get length of document (is -1 when Server sends no Content-Length header)
    int len = httpClient.getSize();

    // create buffer for read
    uint8_t buff[512] = { 0 };
    // read all data from server
    while (httpClient.connected() && (len > 0 || len == -1)) {
      // get available data size
      size_t size = stream->available();
      static TickType_t lastMessageTickCount = xTaskGetTickCount();
      TickType_t diff = xTaskGetTickCount() - lastMessageTickCount;
      printf("diff: %d\n", diff);
      if (diff > (1000 * pingIntervalSeconds)) {
        statusServerConnected = false;
        stream = NULL;
        httpClient.end();
        sseConnected = false;
      }
      if (size) {

        // read up to 512 byte
        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
        lastMessageTickCount = xTaskGetTickCount();
        // write it to Serial
        printf("[Sse] ");
        fwrite(buff, sizeof(char), c, stdout);
        parseRawMessage(buff, c);

        if (len > 0) {
          len -= c;
        }
      } else {
        break;
      }
    }
  };
}