#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char *ssid = "Rasp";
const char *password = "12345678";

// Set LED GPIOs
const int ledUp = 33;
const int ledDown = 32;
const int ledLeft = 26;
const int ledRight = 27;
const int ledBrightness = 25; // LED for brightness control

AsyncWebServer server(80);

// Current brightness level for LED on pin D25
int brightness = 0;
int brightnessStep = 5; // Step for brightness change

// Variables for speed and temperature
int currentSpeed = 0;
int currentTemperature = 25;

void setup() {
    Serial.begin(115200);

    // Initialize LEDs as outputs
    pinMode(ledUp, OUTPUT);
    pinMode(ledDown, OUTPUT);
    pinMode(ledLeft, OUTPUT);
    pinMode(ledRight, OUTPUT);
    pinMode(ledBrightness, OUTPUT);

    // Initialize LED states to HIGH (off)
    digitalWrite(ledUp, HIGH);
    digitalWrite(ledDown, HIGH);
    digitalWrite(ledLeft, HIGH);
    digitalWrite(ledRight, HIGH);

    ledcAttachPin(ledBrightness, 0); // Attach PWM control to pin D25
    ledcSetup(0, 5000, 8); // PWM frequency of 5kHz, 8-bit resolution

    // SPIFFS initialization
    if (!SPIFFS.begin()) {
        Serial.println("Erreur SPIFFS...");
        return;
    }

    // WiFi initialization
    WiFi.begin(ssid, password);
    Serial.print("Tentative de connexion...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("\nConnexion etablie!");
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());

    // Web server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", String(), false);
    });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });

    // SVG image routes
    server.on("/caret-left.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/caret-left.svg", "image/svg+xml");
    });

    server.on("/sort-up.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/sort-up.svg", "image/svg+xml");
    });
    server.on("/square-right.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/square-right.svg", "image/svg+xml");
    });
    server.on("/square-left.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/square-left.svg", "image/svg+xml");
    });
    server.on("/sort-down.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/sort-down.svg", "image/svg+xml");
    });

    server.on("/caret-right.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/caret-right.svg", "image/svg+xml");
    });

    server.on("/square-up.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/square-up.svg", "image/svg+xml");
    });

    server.on("/square-down.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/square-down.svg", "image/svg+xml");
    });

    // Control for each direction button
    server.on("/UP", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received /UP request");
        digitalWrite(ledUp, LOW); // Turn on LED at D33
        request->send(200, "text/plain", "LED UP ON");
    });

    server.on("/DOWN", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received /DOWN request");
        digitalWrite(ledDown, LOW); // Turn on LED at D32
        request->send(200, "text/plain", "LED DOWN ON");
    });

    server.on("/LEFT", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received /LEFT request");
        digitalWrite(ledLeft, LOW); // Turn on LED at D26
        request->send(200, "text/plain", "LED LEFT ON");
    });

    server.on("/RIGHT", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received /RIGHT request");
        digitalWrite(ledRight, LOW); // Turn on LED at D27
        request->send(200, "text/plain", "LED RIGHT ON");
    });

    // Control brightness for LED on D25
    server.on("/BRIGHT_UP", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (brightness < 255) {  // Prevent exceeding max brightness
            brightness = min(brightness + brightnessStep, 255);
            Serial.println("Brightness up: " + String(brightness));
            ledcWrite(0, brightness); 
        }
        request->send(200, "text/plain", String(brightness));
    });

    server.on("/BRIGHT_DOWN", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (brightness > 0) {  // Prevent going below 0
            brightness = max(brightness - brightnessStep, 0);
            Serial.println("Brightness down: " + String(brightness));
            ledcWrite(0, brightness); 
        }
        request->send(200, "text/plain", String(brightness));
    });

    // Off routes for each direction
    server.on("/UP_OFF", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received /UP_OFF request");
        digitalWrite(ledUp, HIGH); // Turn off LED at D33
        request->send(200, "text/plain", "LED UP OFF");
    });

    server.on("/DOWN_OFF", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received /DOWN_OFF request");
        digitalWrite(ledDown, HIGH); // Turn off LED at D32
        request->send(200, "text/plain", "LED DOWN OFF");
    });

    server.on("/LEFT_OFF", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received /LEFT_OFF request");
        digitalWrite(ledLeft, HIGH); // Turn off LED at D26
        request->send(200, "text/plain", "LED LEFT OFF");
    });

    server.on("/RIGHT_OFF", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received /RIGHT_OFF request");
        digitalWrite(ledRight, HIGH); // Turn off LED at D27
        request->send(200, "text/plain", "LED RIGHT OFF");
    });

    // Endpoint to get current speed and temperature
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String jsonResponse = "{\"speed\":" + String(currentSpeed) + ",\"temperature\":" + String(currentTemperature) + "}";
        request->send(200, "application/json", jsonResponse);
    });

    // Endpoint to set speed
    server.on("/setSpeed", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            currentSpeed = request->getParam("value")->value().toInt();
            Serial.println("Speed set to: " + String(currentSpeed));
            request->send(200, "text/plain", "Speed set");
        } else {
            request->send(400, "text/plain", "Missing speed value");
        }
    });

    // Endpoint to set temperature
    server.on("/setTemperature", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            currentTemperature = request->getParam("value")->value().toInt();
            Serial.println("Temperature set to: " + String(currentTemperature));
            request->send(200, "text/plain", "Temperature set");
        } else {
            request->send(400, "text/plain", "Missing temperature value");
        }
    });

    server.begin();
}

void loop() {
}