#include <WiFi.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "thingProperties.h"

// WiFi Credentials
const char* ssid = "Whit gyjfgty";
const char* password = "zxcvbnmm";

// Sensor Pins
#define ONE_WIRE_BUS 25       // DS18B20 data pin
#define TdsSensorPin 34       // ADC1 channel 6 for TDS sensor
#define WaterSensorPin 32     // ADC input for water level sensor

// OneWire and DallasTemperature for DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Local variables to hold sensor values temporarily
float temperature = 25.0;     
float tdsValue = 0.0;
float waterLevelPercentage = 0.0;


void setup() {
  Serial.begin(115200);
  sensors.begin();
  pinMode(WaterSensorPin, INPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Initialize Cloud variables
  initProperties(); // This function is in thingProperties.h
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("Sensor reading started");
}

void loop() {
  ArduinoCloud.update();

  // --- DS18B20 Temperature ---
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature");
    temperature = 25;
  } else {
    temperature = tempC;
  }
  
  // --- TDS Sensor Reading ---
  int tdsRaw = analogRead(TdsSensorPin);
  float voltage = tdsRaw * (3.3 / 4095.0);
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
  float compensatedVoltage = voltage / compensationCoefficient;
  tdsValue = (133.42 * compensatedVoltage * compensatedVoltage * compensatedVoltage
              - 255.86 * compensatedVoltage * compensatedVoltage
              + 857.39 * compensatedVoltage) * 0.5;

  // --- Water Level Sensor ---
  int waterRaw = analogRead(WaterSensorPin);
  waterLevelPercentage = (waterRaw / 4095.0) * 100;

  // --- Assign to Cloud Variables ---
  // Don't redeclare; just assign values to the variables declared in 'thingProperties.h'
  cloudTemperature = temperature;
  cloudTdsValue = tdsValue;
  cloudWaterLevel = waterLevelPercentage;

  // Debug output
  Serial.print("Temp: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("TDS Raw: "); Serial.print(tdsRaw);
  Serial.print(" Voltage: "); Serial.print(voltage, 2);
  Serial.print(" TDS: "); Serial.print(tdsValue, 1); Serial.println(" ppm");
  Serial.print("Water Raw: "); Serial.print(waterRaw);
  Serial.print(" Percentage: "); Serial.print(waterLevelPercentage, 1); Serial.println(" %");
  Serial.println();

  delay(1000);
}
