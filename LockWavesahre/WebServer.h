#include <ESPAsyncWebServer.h>
#include <Update.h>

AsyncWebServer wServer(80);

const char *http_username = "neklepat";
const char *http_password = "ehicehic";

size_t content_len;

const String appVersion = "waveshare 1.0";

const char updateIndex[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
	<title>Aktualizace displeje</title> 
    <meta charset='UTF-8'>

<script>
function _(el) {
  return document.getElementById(el);
}

function uploadFile() {
  var file = _('updateFile').files[0];
  var formdata = new FormData();
  formdata.append('updateFile', file);
  var ajax = new XMLHttpRequest();
  ajax.upload.addEventListener('progress', progressHandler, false);
  ajax.addEventListener('loadend', completeHandler, false);
  ajax.addEventListener('error', errorHandler, false);
  ajax.addEventListener('abort', abortHandler, false);
  ajax.open('POST', '/doUpdate');
  
  ajax.send(formdata);
}

function completeHandler(event) {
  _('progressBar').value = 0;
  _('progressBar').hidden = true;
  _('status').innerHTML = 'Soubor byl úspěšně nahrán';
         
  window.location.href = '/updateDone';
}

function progressHandler(event) {
  if (event.loaded === event.total) {
    completeHandler(event);
  } else {
    var percent = (event.loaded / event.total) * 100;
    _('progressBar').value = Math.round(percent);
    _('progressBar').hidden = false;
    _('status').innerHTML = Math.round(percent) + '%% nahráno... prosím čekejte';
  }
}

function errorHandler(event) {
  _('status').innerHTML = 'Nahrání souboru selhalo';
}

function abortHandler(event) {
  _('status').innerHTML = 'Nahrávání bylo přerušeno';
}

function ajax(elem) {
    var method = elem.getAttribute('method');
    var url = elem.getAttribute('action');
    var data = new FormData(elem);
    xhr = new XMLHttpRequest();
    xhr.open(method, url, true);
    xhr.onload = function(ev) {
      completeHandler(ev); 
    };
    xhr.upload.onprogress = function (ev) {
        progressHandler(ev);
    };
    xhr.send(data);
}

function hookFormSubmit() {
  _('updateForm').addEventListener('submit', function (ev) {
      ajax(this);
      ev.preventDefault();
  }, false);
}
</script>
</head>

<body onload='hookFormSubmit()'>
    <h3>Aktualizace modulu zámků (současná verze FW %appVersion%)</h3>
	<form id='updateForm' method='POST' action='/doUpdate' enctype='multipart/form-data'>
    <input type='file' name='file' id='updateFile'><br><br>
    <progress id="progressBar" value="0" max="100" style="width:300px;" hidden></progress>
    <h4 id="status"></h4>
    <br><br>
    <input type='submit' value='Aktualizovat'>
  </form>
</body>
</html>
)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
	<title>Nastavení zámků</title> 
    <meta charset="UTF-8">
</head>
<body>
    <h3>Nastavení zámků (verze FW %appVersion%)</h3>
	<form  action="/set" method="POST">
    <h4>Vyvolání</h4>

		<label for="serverName">Server</label>
		<br>
        <input type="text" id="serverName" name="serverName" value="%serverName%"/>
		<br><br>
        <label for="nodeId">Id obrazovky (ponechte prázdné pokud používáte pouze jednu skupinu displejů) </label>
		<br>
        <input type="text" id="nodeId" name="nodeId" value="%nodeId%"/> 
    <br><br>
        <label for="unlockTime">Doba otevření zámku v sekundách </label>
		<br>
        <input type="unlockTime" id="unlockTime" name="unlockTime" value="%unlockTime%" min="1" max="60"/> 
    <br><br>

    <h4>LAN</h4>

		<label for="displayIP">IP adresa (ponechte prázdné pokud chcete přidělit automaticky)</label>
		<br>
        <input type="text" id="displayIP" name="displayIP" value="%displayIP%"/>
		<br><br>
        <label for="subnetMask">Maska podsítě </label>
		<br>
        <input type="text" id="subnetMask" name="subnetMask" value="%subnetMask%"/> 
        <br><br>
        <label for="gatewayIP">IP adresa brány </label>
		<br>
        <input type="text" id="gatewayIP" name="gatewayIP" value="%gatewayIP%"/> 
        <br><br>    
        <label for="dnsIP">IP adresa DNS serveru </label>
		<br>
        <input type="text" id="dnsIP" name="dnsIP" value="%dnsIP%"/> 
        <br>
        <br>  
      <label>
     
  Skrýt IP adresu
</label>
        <br>
        <br> 
        <input type="submit" value="Uložit">
    </form>
    <br>
    <br>
    <a href="/update">Aktualizace firmware</a>

</body>
</html>
)rawliteral";

const char set_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
	<title>Nastavení displeje</title> 
    <meta charset="UTF-8">
</head>
<body>
    <h3>Nastavení bylo uloženo</h3><br><br>
	<form  action="/" method="GET">  
     <input type="submit" value="Zpět">
  </form>
</body>
</html>
)rawliteral";

const char updateDone[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
	<title>Aktualizace dokončena</title> 
    <meta charset="UTF-8">
</head>
<body>
    <h3>Aktualizace FW byla dokončena</h3>
    Zařízení se nyní restartuje...<br><br>
	<form  action="/" method="GET">  
     <input type="submit" value="Zpět">
  </form>
</body>
</html>
)rawliteral";

// Replaces placeholder with stored values
String processor(const String &var) {
  //Serial.println("Process placeholder " + var);
  if (var == "appVersion") {
    return appVersion;
  } else if (var == "serverName") {
    return serverName;
  } else if (var == "nodeId") {
    if (nodeId == "node_tabule") {
      return "";
    } else {
      return nodeId;
    }
  } else if (var == "displayIP") {
    return displayIP;
  } else if (var == "gatewayIP") {
    return gatewayIP;
  } else if (var == "subnetMask") {
    return subnetMask;
  } else if (var == "dnsIP") {
    return dnsIP;
  } else if (var == "unlockTime") {
    return String(unlockTime);
  }

  return String();
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", index_html, processor);
}

void printProgress(size_t prg, size_t sz) {
  Serial.printf("Update progress: %d%%\n", (prg * 100) / content_len);
}

void handleUpdateIndex(AsyncWebServerRequest *request) {
  Update.onProgress(printProgress);
  request->send_P(200, "text/html", updateIndex, processor);
}

void handleUpdateDone(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", updateDone, processor);
}

void handleSet(AsyncWebServerRequest *request) {
  if (request->hasParam("serverName", true)) {
    const AsyncWebParameter *p = request->getParam("nodeId", true);
    nodeId = p->value().c_str();
    if (nodeId == "") {
      // default nodeId
      nodeId = "node_tabule";
    }

    p = request->getParam("serverName", true);
    serverName = p->value().c_str();

    p = request->getParam("displayIP", true);
    displayIP = p->value().c_str();
    p = request->getParam("gatewayIP", true);
    gatewayIP = p->value().c_str();
    p = request->getParam("subnetMask", true);
    subnetMask = p->value().c_str();
    p = request->getParam("dnsIP", true);
    dnsIP = p->value().c_str();
    p = request->getParam("unlockTime", true);
    String unlockTimeStr = p->value().c_str();
    if (unlockTimeStr != "") {
      unlockTime = unlockTimeStr.toInt();
      if (unlockTime > 60) {
        unlockTime = 60;
      } else 
      if (unlockTime < 1){
        unlockTime = 1;
      }
    }

    
    int params = request->params();
  
    storeServerSettings();
  }

  request->send_P(200, "text/html", set_html, processor);
}

void handleDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {

  if (!index) {
    printf("Update");
    content_len = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = (filename.indexOf("spiffs") > -1) ? U_SPIFFS : U_FLASH;

    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  if (final) {
    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      printf("Aktualizace byla dokončena 1");
      Serial.flush();

      request->send_P(200, "text/html", updateDone, processor);

      delay(1000);
      printf("Aktualizace byla dokončena 2");
      forceSystemReset();
    }
  }
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404 - Stránka nenalezena");
}

void setupServer() {

  // https://myhomethings.eu/en/esp32-asynchronous-web-server/

  wServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    handleRoot(request);
  });

  wServer.on("/set", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    handleSet(request);
    forceSystemReset();
  });

  wServer.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    handleUpdateIndex(request);
  });

  wServer.on("/updateDone", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    handleUpdateDone(request);
  });

  wServer.on(
    "/doUpdate", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      printf("Request ???");
    },
    [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data,
       size_t len, bool final) {
      handleDoUpdate(request, filename, index, data, len, final);
    });

  wServer.onNotFound(handleNotFound);

  wServer.begin();
  printf("HTTP server started\n");
}

void loopServer() {
}