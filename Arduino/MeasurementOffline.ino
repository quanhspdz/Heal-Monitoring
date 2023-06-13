#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);
#include "MAX30100_PulseOximeter.h"
#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_TEMPLATE_ID "TMPL6-YWqewxY"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "C8xFOM6wl0p9qeTouRbWPuwM5YeqlhSi"
#define BLYNK_PRINT Serial

char ssid[] = "Ai đẹp trai nhất nhà";
char pass[] = "quanganh";

uint32_t tsLastReport = 0;
void onBeatDetected(){
    Serial.println("Beat!");
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
    display.println("Welcom to");
    display.setCursor(0,40);             
    display.println("Heal Measure System");
    display.display();
    display.setTextSize(1);
    delay(2000); 
    Serial.print("Initializing pulse oximeter..");

    // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
    pox.setOnBeatDetectedCallback(onBeatDetected);
}
void loop(){
    pox.update();
    // Blynk.run();

    uint32_t heartRate = pox.getHeartRate();
    uint32_t spo2 = pox.getSpO2();

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
        tsLastReport = millis();

        // Blynk.virtualWrite(V4, heartRate);
        // Blynk.virtualWrite(V5, spo2);    
    }
}
