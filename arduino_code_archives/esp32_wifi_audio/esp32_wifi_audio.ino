#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

// Replace with your network credentials
//const char* ssid = "sara";
//const char* password = "weeniehut";

const char* ssid = "chrisc";
const char* password = "12345678";

WebServer server(80);

// Handle the audio file request
void handleAudio() {
    File audioFile = SPIFFS.open("/audio_test.mp3", "r");
    if (!audioFile) {
        server.send(404, "text/plain", "Audio file not found!");
        return;
    }

    server.streamFile(audioFile, "audio/mpeg");
    audioFile.close();
}

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS!");
        return;
    }

    // Set up server routes
    server.on("/audio", handleAudio);
    server.begin();
    Serial.println("Web server started!");
}

void loop() {
    server.handleClient();
}
