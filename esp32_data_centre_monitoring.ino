#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

// Sensor Pins
#define DHTPIN 2
#define DHTTYPE DHT22
#define PIRPIN 17
#define LDRPIN 34

DHT dht(DHTPIN, DHTTYPE);

// WiFi & Free Public MQTT Broker Setup
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP32Client-DataCenter-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PIRPIN, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Read Sensors
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int motion = digitalRead(PIRPIN);
  int light = analogRead(LDRPIN);

  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Edge Intelligence Logic
  String risk = "Low";
  String coolingRec = "Eco Mode";

  if (t > 30.0 && h > 60.0 && motion == 0) {
    risk = "High";
    coolingRec = "Emergency AC + Dehumidifier";
  } else if (t > 26.0 && motion == 1 && light < 2000) {
    risk = "Medium";
    coolingRec = "Increase Airflow - Zone 1";
  } else if (motion == 0 && light < 2000) {
    risk = "Low - Energy Waste";
    coolingRec = "Eco Mode (Alert: Lights On)";
  }

  // Construct JSON Payload
  StaticJsonDocument<200> doc;
  doc["temperature"] = t;
  doc["humidity"] = h;
  doc["light"] = light;
  doc["motion"] = motion;
  doc["risk"] = risk;
  doc["cooling_recommendation"] = coolingRec;

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  // Publish to MQTT Topic
  Serial.print("Publishing message: ");
  Serial.println(jsonBuffer);
  client.publish("sit/datacenters/sensorData", jsonBuffer);

  delay(5000);
}
