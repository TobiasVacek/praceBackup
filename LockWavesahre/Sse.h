#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Settings.h>

String lastSseServer = "";

HTTPClient httpClient;
WiFiClient * stream = NULL;

void parseSseMessage(String type, uint8_t* payload) {
  printf("[Sse] get text (%s): %s\n", type, payload);

  DynamicJsonDocument doc(500);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    printf("deserializeJson() failed: ");
    printf("%s\n", error.f_str());
    return;
  } else {
    String cliId = doc["clientId"];
    String action = doc["action"];
    
    //TODO    
  }
}

boolean checkConnected() {
  String url = "http://192.168.2.103:8008/api/sse/listen";
  boolean justConnected = false;
  if (!httpClient.connected()) {
    printf("[Sse] connecting SSE at: ");
    printf("%s\n", url);
    
    httpClient.begin(url);
    justConnected = true;
  }

  if (httpClient.connected() || justConnected) {
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

          // create buffer for read
          uint8_t buff[128] = { 0 };

          // get tcp stream
          stream = httpClient.getStreamPtr();
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

      if (size) {
        // read up to 512 byte
        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

        // write it to Serial
        printf("[Sse] ");
        Serial.write(buff, c);

        if (len > 0) {
            len -= c;
        }
      } else {
        break;
      }
    }

  };

}