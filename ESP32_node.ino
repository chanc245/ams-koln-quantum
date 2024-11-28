#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient_Generic.h>

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

// WiFi and WebSocket details
const char* ssid = "chrisc";
const char* password = "12345678";
#define WS_SERVER "ip"
#define WS_PORT 8080

bool alreadyConnected = false;

// Initialize Serial2 for Arduino communication
#define RX2 16
#define TX2 17

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("[WSc] Disconnected!");
      alreadyConnected = false;
      break;

    case WStype_CONNECTED:
      Serial.print("[WSc] Connected to server: ");
      Serial.println((char*)payload);
      alreadyConnected = true;
      break;

    case WStype_TEXT:
      Serial.printf("[WSc] Received message: %s\n", payload);
      break;

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200); // Debug Serial
  Serial2.begin(115200, SERIAL_8N1, RX2, TX2); // Setup Serial2 for Arduino communication
//  Serial.println("Serial2 initialized for Arduino communication.");

  // WiFi setup
  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // WebSocket setup
  webSocket.begin(WS_SERVER, WS_PORT, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();

  // Read messages from Arduino
  if (Serial2.available()) {
    String arduinoMessage = Serial2.readStringUntil('\n'); // Read message until newline
    arduinoMessage.trim(); // Remove any leading/trailing whitespace or control characters

    // Debug raw and trimmed input
//    Serial.print("Raw message from Arduino: '");
//    Serial.print(arduinoMessage);
//    Serial.println("'");

    // Validate the message: Only accept single-digit numbers
    if (arduinoMessage.length() == 1 && isdigit(arduinoMessage[0])) {
      Serial.print("msg from Arduino: ");
      Serial.println(arduinoMessage);

      // Example: Send the number to WebSocket server
      if (alreadyConnected) {
        webSocket.sendTXT(arduinoMessage);
      }
    } else {
      Serial.println("Invalid message received. Ignoring.");
    }

    // Clear the serial buffer
    Serial2.flush();
  }

  // Example: Send data to the WebSocket server periodically
  static unsigned long lastTime = 0;
  static String lastArduinoMessage = ""; // Store the last received single-digit number

  if (millis() - lastTime > 5000) {
    lastTime = millis();
    if (alreadyConnected && lastArduinoMessage.length() == 1) {
      webSocket.sendTXT("Arduino Message: " + lastArduinoMessage);
      Serial.println("[WSc] Sent: " + lastArduinoMessage);
    }
  }

}
