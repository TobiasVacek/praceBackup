#include <HTTPClient.h>
#include <ArduinoJson.h>
//#include <Settings.h>

String lastSseServer = "";

HTTPClient httpClient;
WiFiClient* stream = NULL;
const String terminatingChar = "\n\n\r\n";

String returnSubstring(String message, String fromString, String toString, boolean includeEdges = false) {
  //returns substring of message between two given Strings
  if (includeEdges) {
    return message.substring(message.indexOf(fromString), message.indexOf(toString) + 1);
  }
  return message.substring(message.indexOf(fromString) + (sizeof(fromString) / sizeof(char)), message.indexOf(toString));
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
      /*printf("-----------------------\n");
      printf("message: %s\n", message.substring(0, endIndex).c_str());
      printf("////////////////////////////\npartial message: %s", partialMessage.c_str());
      printf("-----------------------\n");*/
      partialMessage = message.substring(endIndex + sizeof(terminatingChar) / sizeof(char), size);
      message = message.substring(0, endIndex);

      if (message.indexOf("id:") != -1) {
        id = returnSubstring(message, "id:", "\n").toInt();
        printf("id is: %d\n", id);
      }
      if (message.indexOf("event:") != -1) {
        event = returnSubstring(message, "event:", "\n");
        printf("parsed event = %s\n", event.c_str());
      }
      if (message.indexOf("data:") != -1) {
        data = returnSubstring(message, "{", "}",true);
        printf("parsed data %s\n", data.c_str());
      }
      message = "";

    } else {
      partialMessage = message;
      message = "";
      recievedWholeMessage = false;
    }
  }
}


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

  String url = serverName + "/api/sse/listen";

  boolean justConnected = false;
  if (!httpClient.connected()) {
    printf("[Sse] connecting SSE at: ");
    printf("%s\n", url.c_str());

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
        fwrite(buff, sizeof(char), c, stdout);


        for (int i = 0; i <= c; i++) {  //only for testing
          printf("%0x ", buff[i]);
        }
        putchar('\n');
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