#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi credentials
const char* apSSID = "Test";
const char* apPassword = "bilalakhtar";

unsigned long lastStatusTime = 0;
int previousClientCount = 0;
unsigned long requestCount = 0;


// XIAO ESP32S3 built-in LED
#define LED_PIN 21

// PWM settings
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

WebServer server(80);

int brightness = 50;

// Set LED brightness (0-100%)

void setBrightness(int value) {
  int previousBrightness = brightness;

  brightness = constrain(value, 0, 100);

  // Active-LOW LED
  int duty = map(brightness, 0, 100, 255, 0);
  ledcWrite(LED_PIN, duty);

  Serial.println("--------------------------------");
  Serial.printf("[LED] Brightness: %d%% -> %d%%\n",
                previousBrightness, brightness);
  Serial.printf("[PWM] Duty: %d / 255\n", duty);
  Serial.printf("[TIME] Uptime: %lu ms\n", millis());
}


// Web page
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport"
        content="width=device-width, initial-scale=1">
  <title>ESP32S3 LED Controller</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #101827;
      color: white;
      text-align: center;
      padding: 35px 15px;
    }
    .card {
      max-width: 400px;
      margin: auto;
      padding: 30px;
      background: #1e293b;
      border-radius: 20px;
    }
    h2 { color: #facc15; }
    .value {
      font-size: 48px;
      margin: 25px;
      color: #facc15;
    }
    input[type=range] {
      width: 100%;
      accent-color: #facc15;
      cursor: pointer;
    }
    button {
      padding: 12px 25px;
      margin: 10px 5px;
      border: none;
      border-radius: 8px;
      background: #facc15;
      color: #101827;
      cursor: pointer;
      font-weight: bold;
    }
  </style>
</head>
<body>

<div class="card">
  <h2>ESP32S3 LED Control</h2>
  <p>Built-in LED Brightness</p>

  <div class="value">
    <span id="value">%VALUE%</span>%
  </div>

  <input type="range" min="0" max="100"
         value="%VALUE%" id="slider"
         oninput="updateBrightness(this.value)">

  <p>0% - 100%</p>

  <button onclick="setLED(0)">OFF</button>
  <button onclick="setLED(100)">ON</button>
</div>

<script>
let pending;

function updateBrightness(value) {
  document.getElementById("value").innerText = value;

  clearTimeout(pending);
  pending = setTimeout(() => {
    fetch("/brightness?value=" + value)
      .catch(err => console.error(err));
  }, 30);
}

function setLED(value) {
  document.getElementById("slider").value = value;
  updateBrightness(value);
}
</script>

</body>
</html>
)rawliteral";

// Main page
void handleRoot() {
  String page = webpage;
  page.replace("%VALUE%", String(brightness));
  server.send(200, "text/html", page);
}

// Brightness API

void handleBrightness() {
  requestCount++;

  Serial.printf("[HTTP] Request #%lu\n", requestCount);

  Serial.printf("[HTTP] Client IP: %s\n",
                server.client().remoteIP().toString().c_str());

  if (!server.hasArg("value")) {
    Serial.println("[ERROR] Missing brightness value");
    server.send(400, "text/plain", "Missing value");
    return;
  }

  int value = server.arg("value").toInt();
  setBrightness(value);

  Serial.println("[HTTP] Response: 200 OK");
  server.send(200, "text/plain", String(brightness));
}



//////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("================================");
  Serial.println("  XIAO ESP32S3 LED CONTROLLER");
  Serial.println("  AP MODE + SERIAL MONITORING");
  Serial.println("================================");

  Serial.println("[SYSTEM] Initializing LED PWM...");

  if (!ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION)) {
    Serial.println("[ERROR] PWM initialization failed!");
    return;
  }

  setBrightness(50);

  Serial.println("[WIFI] Starting Access Point...");

  WiFi.mode(WIFI_AP);

  IPAddress localIP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  if (!WiFi.softAPConfig(localIP, gateway, subnet)) {
    Serial.println("[ERROR] AP IP configuration failed");
    return;
  }

  if (!WiFi.softAP(apSSID, apPassword)) {
    Serial.println("[ERROR] AP initialization failed!");
    return;
  }

  Serial.println("[WIFI] Access Point started!");
  Serial.printf("[WIFI] SSID: %s\n", apSSID);
  Serial.printf("[WIFI] IP Address: %s\n",
                WiFi.softAPIP().toString().c_str());
  Serial.printf("[WIFI] MAC: %s\n",
                WiFi.softAPmacAddress().c_str());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/brightness", HTTP_GET, handleBrightness);

  server.begin();

  Serial.println("[SERVER] HTTP server started");
  Serial.println("[SERVER] Listening on port 80");
  Serial.println("[READY] http://192.168.4.1");
  Serial.println("================================");
}


void loop() {
  server.handleClient();

  int clientCount = WiFi.softAPgetStationNum();

  if (clientCount != previousClientCount) {
    if (clientCount > previousClientCount) {
      Serial.println("[WIFI] Client connected");
    } else {
      Serial.println("[WIFI] Client disconnected");
    }

    Serial.printf("[WIFI] Connected clients: %d\n",
                  clientCount);

    previousClientCount = clientCount;
  }

  // Periodic system monitoring
  if (millis() - lastStatusTime >= 5000) {
    lastStatusTime = millis();

    Serial.println();
    Serial.println("======== SYSTEM STATUS ========");

    Serial.printf("Uptime: %lu seconds\n",
                  millis() / 1000UL);

    Serial.printf("Connected Clients: %d\n",
                  WiFi.softAPgetStationNum());

    Serial.printf("LED Brightness: %d%%\n", brightness);

    Serial.printf("Free Heap: %u bytes\n",
                  ESP.getFreeHeap());

    Serial.printf("HTTP Requests: %lu\n",
                  requestCount);

    Serial.println("===============================");
  }
}
