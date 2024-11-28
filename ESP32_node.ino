#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient_Generic.h>

#if !defined(ESP32)
  #error This code is intended to run only on the ESP32 boards! Please check your Tools->Board setting.
#endif

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SSL false  // Set to true if your Node.js server uses SSL/TLS

// WiFi credentials
const char* ssid = "chrisc";
const char* password = "12345678";

// Node.js server details
#define WS_SERVER "ip adress"  // Replace with the IP of your Node.js server
#define WS_PORT 8080  // Replace with your Node.js server port

bool alreadyConnected = false;

// WebSocket event handler
void webSocketEvent(const WStype_t& type, uint8_t *payload, const size_t& length) {
  switch (type) {
    case WStype_DISCONNECTED:
      if (alreadyConnected) {
        Serial.println("[WSc] Disconnected!");
        alreadyConnected = false;
      }
      break;

    case WStype_CONNECTED:
      alreadyConnected = true;
      Serial.print("[WSc] Connected to server: ");
      Serial.println((char*)payload);
      break;

    case WStype_TEXT:
      Serial.printf("[WSc] Received message: %s\n", payload);
      break;

    case WStype_BIN:
      Serial.printf("[WSc] Received binary message of length: %u\n", length);
      break;

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("\nStarting ESP32 WebSocket Client...");

  // Connect to WiFi
  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Node website adress: localhost:");
  Serial.println(WS_PORT);

  // Connect to WebSocket server
#if USE_SSL
  webSocket.beginSSL(WS_SERVER, WS_PORT, "/");
#else
  webSocket.begin(WS_SERVER, WS_PORT, "/");
#endif

  // Attach event handler
  webSocket.onEvent(webSocketEvent);

  // Reconnect every 5 seconds 
  webSocket.setReconnectInterval(5000);

  Serial.println("WebSocket setup complete.");
}

void loop() {
  webSocket.loop();

  static unsigned long lastTime = 0;
  if (millis() - lastTime > 5000) {
    lastTime = millis();
    if (alreadyConnected) {
      webSocket.sendTXT("1");
      Serial.println("[WSc] Sent: 1");
    }
  }
}
