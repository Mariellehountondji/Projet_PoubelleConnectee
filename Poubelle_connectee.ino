
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Servo.h>
const char* ssid = "youpilab_fibre 2G";
const char* password = "Washingi_loV3_yl2025Fibre#Cit1";

const String BASE_URL = "https://iot.youpilab.com/api";
const String APP_ID   = "pouf5596";
const String APP_KEY  = "9881b465";

// ================= BROCHES =================
#define TRIG_PIN 13  // D7
#define ECHO_PIN 15  // D8
#define SERVO_PIN 12 // D6

Servo monServo;

// ================= VARIABLES =================
bool plateformeOuverte = false;
unsigned long lastCheckTime = 0;

// ================= WIFI =================
void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté !");
}

// ================= API =================
void checkControls() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String url = BASE_URL + "/controls/get?APP_ID=" + APP_ID + "&APP_KEY=" + APP_KEY;
  
  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        String lastValue = "";
        // Traitement du tableau reçu (prend le dernier élément)
        if (doc.is<JsonArray>()) {
          JsonArray arr = doc.as<JsonArray>();
          if (arr.size() > 0) {
            lastValue = arr[arr.size() - 1].as<String>();
          }
        } else {
          lastValue = doc.as<String>();
        }
        
        plateformeOuverte = (lastValue == "1");
        Serial.print("Derniere valeur API : ");
        Serial.println(lastValue);
      }
    }
    http.end();
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  monServo.attach(SERVO_PIN);
  monServo.write(10); // Position fermée initiale
  
  setup_wifi();
}

// ================= LOOP =================
void loop() {
  // 1. Mise à jour API toutes les 5 secondes
  if (millis() - lastCheckTime > 500) {
    checkControls();
    lastCheckTime = millis();
  }

  // 2. Mesure Ultrason
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  int distance = (duration > 0) ? duration / 58 : 999;

  // 3. Logique hybride (OU logique)
  // Ouvre si la plateforme demande OU si une présence est détectée
  if (plateformeOuverte || (distance > 0 && distance < 30)) {
    monServo.write(120); 
  } else {
    monServo.write(10);
  }
  
  delay(100);
}