#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <limits.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);
#include "MAX30100_PulseOximeter.h"
#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_TEMPLATE_ID "TMPL6-YWqewxY"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "C8xFOM6wl0p9qeTouRbWPuwM5YeqlhSi"
#define BLYNK_PRINT Serial

char ssid[] = "Quang Anh";
char pass[] = "12345678";

char supSSID[] = "Ai đẹp trai nhất nhà";
char supPASS[] = "quanganh";

int port = 80;
WiFiServer server(port);
WiFiClient client;

uint32_t tsLastReport = 0;

const int ledPin = 2; // GPIO pin connected to LED
const int buzzerPin = 16; // GPIO pin connected to BUZZER
const int buzzerFreq = 3500;
int alarmRing = 0;

void onBeatDetected(){
    Serial.println("Beat!");

    noTone(buzzerPin);
    tone(buzzerPin, buzzerFreq, 200);
}

void displayDataOLED(uint32_t heartRate, uint32_t spo2) {
   if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(heartRate);
        Serial.print("bpm / SpO2:");
        Serial.print(spo2);
        Serial.println("%");

        display.clearDisplay(); 
        display.setCursor(10,12); 
        display.print("Pulse Oximeter");     
        display.setCursor(0,35); 
        display.print("HeartR:");
        display.setCursor(62,35);
        display.print(heartRate); 
        display.println(" bpm");
        display.setCursor(0,59);
        display.print("SpO2  : ");
        display.setCursor(62,59);
        display.print(spo2);
        display.println(" %");
        display.display();

        if (!alarmRing && heartRate == 0) {
          digitalWrite(buzzerPin, HIGH);
          alarmRing = 1;
        } else if (alarmRing && heartRate > 0) {
          digitalWrite(ledPin, LOW);
          alarmRing = 0;
        }

        tsLastReport = millis();
   }
}

void connectToWiFiAndBlynk() {
  Serial.print("Đang kết nối với ");
  Serial.print(ssid);
  WiFi.begin(ssid, pass);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 5) {
    delay(1000);
    retries++;
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Kết nối thành công với ");
    Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    return;
  }

  Serial.print("Không thể kết nối với ");
  Serial.println(ssid);

  Serial.print("Đang kết nối với ");
  Serial.print(supSSID);
  WiFi.begin(supSSID, supPASS);

  retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 5) {
    delay(1000);
    retries++;
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Kết nối thành công với ");
    Serial.println(supSSID);
    Blynk.begin(BLYNK_AUTH_TOKEN, supSSID, supPASS);
  } else {
    Serial.print("Không thể kết nối với ");
    Serial.println(supSSID);
  }
}

void setup(){
    Serial.begin(115200);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  
      Serial.println("SSD1306 allocation failed");
      for(;;);
    }
    display.setFont(&FreeSerif9pt7b);
    display.clearDisplay();
    display.setTextSize(1);             
    display.setTextColor(WHITE);        
    display.setCursor(20,15);             
    display.println("Welcome to");
    display.setCursor(0,40);             
    display.println("Heal Measure System");
    display.display();
    display.setTextSize(1);
    delay(2000); 
    Serial.print("Initializing pulse oximeter..");

    connectToWiFiAndBlynk();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".....");
      delay(500);
    }

    server.begin();
    Serial.print("Open Telnet and connect to IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" on port ");
    Serial.println(port);

    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
    pox.setOnBeatDetectedCallback(onBeatDetected);

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH); // Turn off LED

    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW); 
}

void sendHeartRateToClient(WiFiClient& client) {
  int heartRate = pox.getHeartRate();
  client.println(heartRate);
}

void sendSpO2ToClient(WiFiClient& client) {
  int spo2 = pox.getSpO2();
  client.println(spo2);
}

void handleClientRequest(WiFiClient& client) {
  uint32_t request = client.read();

  switch (request) {
    case 'h': //'h' is heart-rate
      Serial.println("Sending heart-rate to client...");
      sendHeartRateToClient(client);
      break;
    case 's': //'s' is SpO2
      Serial.println("Sending SpO2 to client...");
      sendSpO2ToClient(client);
      break;
  }
}

void loop(){
  pox.update();
  Blynk.run();

  uint32_t heartRate = pox.getHeartRate();
  uint32_t spo2 = pox.getSpO2();

  displayDataOLED(heartRate, spo2);

  Blynk.virtualWrite(V4, heartRate);
  Blynk.virtualWrite(V5, spo2);  

  if (!client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("Client connected");
    }
  } else {
    handleClientRequest(client);
  }
}
