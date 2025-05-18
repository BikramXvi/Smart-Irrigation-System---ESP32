#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>

// Blynk configuration
#define BLYNK_TEMPLATE_ID "TMPL6muJQPCLA"
#define BLYNK_TEMPLATE_NAME "SmartIrrigation"
#define BLYNK_AUTH_TOKEN "3JqqxdX_xFREC85TToDCw-vtkrwmJMAu"

#include <BlynkSimpleEsp32.h>

// WiFi credentials
char ssid[] = "JOJOWIFI";
char pass[] = "Kanxi@123";

// Pin setup
#define SOIL_PIN 35
#define RELAY_PIN 27
#define DHT_PIN 4
#define TRIG_PIN 12
#define ECHO_PIN 34
#define BUZZER_PIN 25

// Thresholds
#define MOISTURE_THRESHOLD 3000
#define LOW_WATER_LEVEL 15.0
#define TEMP_THRESHOLD 25.0  // Temperature must be > 25°C to allow watering

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT11);

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.init();
  lcd.clear();
  lcd.backlight();

  dht.begin();
}

void loop() {
  Blynk.run();

  int soilMoisture = analogRead(SOIL_PIN);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Ultrasonic reading
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
//   if (duration == 0) {
//   duration = 30000; // fake value, prevents crash
// }
  float waterLevel = (duration * 0.0343) / 2;

  // Pump control with temp + soil condition
  String pumpStatus;
  if (soilMoisture > MOISTURE_THRESHOLD || temperature > TEMP_THRESHOLD) {
    digitalWrite(RELAY_PIN, LOW);  // Pump ON
    pumpStatus = "Pump ON - Dry & Hot";
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // Pump OFF
    pumpStatus = "Pump OFF - No need";
  }

  // Buzzer alert for low water
  if (waterLevel > LOW_WATER_LEVEL) {
    Blynk.logEvent("low_water_level");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }

  // Send sensor values to Blynk
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V3, soilMoisture);
  Blynk.virtualWrite(V4, waterLevel);
  Blynk.virtualWrite(V8, pumpStatus); // Pump status to V8

  // Debug output
  Serial.print("Soil: ");
  Serial.print(soilMoisture);
  Serial.print(" | Temp: ");
  Serial.print(temperature);
  Serial.print("C");
  Serial.print(" | Humidity: ");
  Serial.print(humidity);
  Serial.print("%");
  Serial.print(" | Water: ");
  Serial.print(waterLevel);
  Serial.println("cm");

  // LCD Display - Line 1: Temp and Humidity
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print((int)temperature);
  lcd.print("C H:");
  lcd.print((int)humidity);
  lcd.print("% ");

  // Soil Status
  String soilStatus;
  if (soilMoisture > 3000) {
    soilStatus = "Dry ";
  } else if (soilMoisture > 2000) {
    soilStatus = "Mod ";
  } else {
    soilStatus = "Wet ";
  }

  // Friendly soil moisture status
  String soilMessage;
  if (soilMoisture > 3000) {
    soilMessage = " Soil is Dry ";
  } else if (soilMoisture > 2000) {
    soilMessage = " Soil is Moderate";
  } else {
    soilMessage = " Soil is Wet";
  }

  // Water Level Status
  String waterStatus;
  if (waterLevel < 5) {
    waterStatus = "Full";
  } else if (waterLevel < 15) {
    waterStatus = "Half";
  } else {
    waterStatus = "Empty";
  }

  // Friendly water level status
  String waterMessage;
  if (waterLevel < 5) {
    waterMessage = "Water Tank Full";
  } else if (waterLevel < 15) {
    waterMessage = "Water Half - Monitor";
  } else {
    waterMessage = "Water Tank Empty - Refill Now";
  }

  // Send status messages to Blynk
  Blynk.virtualWrite(V6, soilMessage);
  Blynk.virtualWrite(V7, waterMessage);

  // LCD Display - Line 2: Soil and Water Status
  lcd.setCursor(0, 1);
  lcd.print("SM:");
  lcd.print(soilStatus);
  lcd.print(" WL:");
  lcd.print(waterStatus);

  delay(2000);
}
