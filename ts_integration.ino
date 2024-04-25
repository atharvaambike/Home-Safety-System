#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define WIFI_SSID "ath"    // Your network SSID (name)
#define WIFI_PASSWORD "athathath"
#define THINGSPEAK_API_KEY "HGLOSQ4ER58JQYRM" // Your ThingSpeak API Key

#define LED_PIN    13 // GPIO13 connected to LED's pin (corresponds to D7 on NodeMCU)
#define BUZZER_PIN 12 // GPIO12 connected to Buzzer (you can choose any available GPIO pin)
#define DISTANCE_THRESHOLD 5 // centimeters
#define SMOKE_THRESHOLD 500 // Analog value threshold for smoke detection
#define TRIG_PIN   5 // GPIO5 connected to Ultrasonic Sensor's TRIG pin (corresponds to D1 on NodeMCU)
#define ECHO_PIN   4 // GPIO4 connected to Ultrasonic Sensor's ECHO pin (corresponds to D2 on NodeMCU)
#define AO_PIN A0  // esp8266's pin connected to AO pin of the MQ2 sensor

float duration_us, distance_cm;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT); // Configure the ESP8266 pin to the output mode
  pinMode(ECHO_PIN, INPUT);  // Configure the ESP8266 pin to the input mode

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Ultrasonic Sensor code
    // Produce a 10-microsecond pulse to the TRIG pin.
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the pulse duration from the ECHO pin
    duration_us = pulseIn(ECHO_PIN, HIGH);
    // calculate the distance
    distance_cm = 0.017 * duration_us;

    Serial.print("Distance: ");
    Serial.println(distance_cm);

    if (distance_cm < DISTANCE_THRESHOLD) {
      digitalWrite(LED_PIN, HIGH); // turn on LED
    } else {
      digitalWrite(LED_PIN, LOW);  // turn off LED
    }

    // Gas Sensor code
    int gasValue = analogRead(AO_PIN);
    Serial.print("AO value: ");
    Serial.println(gasValue);

    if (gasValue > SMOKE_THRESHOLD) {
      digitalWrite(BUZZER_PIN, LOW); // turn on buzzer
    } else {
      digitalWrite(BUZZER_PIN, HIGH); // turn off buzzer
    }

    // Send data to ThingSpeak
    sendDataToThingSpeak(distance_cm, gasValue);

    delay(2000); // Reduce delay to send data more frequently
  } else {
    Serial.println("WiFi Disconnected");
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW); // turn off buzzer
    delay(500);
  }
}

void sendDataToThingSpeak(float distance_cm, int gasValue) {
  if (client.connect("api.thingspeak.com", 80)) {
    String postStr = String(THINGSPEAK_API_KEY);
    postStr += "&field1=";
    postStr += String(distance_cm);
    postStr += "&field2=";
    postStr += String(gasValue);
    postStr += "\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + String(THINGSPEAK_API_KEY) + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();
}
