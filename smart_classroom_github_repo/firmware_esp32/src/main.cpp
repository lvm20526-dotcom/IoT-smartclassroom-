#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_INA219.h>

// ==================== CẤU HÌNH MẠNG & HIVEMQ ====================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MQTT_BROKER    = "broker.hivemq.com";
const int   MQTT_PORT      = 1883;
const char* MQTT_CLIENT_ID = "ESP32_SmartClassroom_Room9";

const char* TOPIC_TELEMETRY = "classroom/room9/telemetry";
const char* TOPIC_STATUS    = "classroom/room9/status";
const char* TOPIC_COMMAND   = "classroom/room9/command";

// ==================== MAPPING CHÂN I/O ====================
const int PIN_PIR         = 12; // PIR HC-SR501
const int PIN_RADAR       = 13; // Radar RCWL-0516
const int PIN_DHT         = 14; // DHT22
const int PIN_RELAY_LIGHT = 25; // Relay Đèn
const int PIN_RELAY_FAN   = 26; // Relay Quạt

#define DHT_TYPE DHT22

// Ngưỡng Hysteresis
const float TEMP_THRESHOLD_ON   = 28.0;
const float TEMP_HYSTERESIS_GAP  = 1.0;
const float LUX_THRESHOLD_ON    = 100.0;
const float LUX_HYSTERESIS_GAP   = 15.0;
const unsigned long TIME_PIR_TIMEOUT_MS = 60000;

// Biến hệ thống
bool isOccupied = false;
bool isAutoMode = true;
bool lightState = false;
bool fanState   = false;

unsigned long lastMotionTime    = 0;
unsigned long lastTelemetryTime = 0;
unsigned long lastEnergyCalcTime = 0;
double accumulatedEnergyKwh    = 0.0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHT dhtSensor(PIN_DHT, DHT_TYPE);
BH1750 lightMeter;
Adafruit_INA219 ina219;

void setupWiFi();
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void readSensors(float &temp, float &hum, float &lux);
void updateOccupancyState();
void processHysteresisControl(float temp, float lux);
void applyActuators(bool lightTarget, bool fanTarget);
void calculateEnergyUsage();
void publishTelemetry(float temp, float hum, float lux);

void setup() {
    Serial.begin(115200);

    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_RADAR, INPUT);
    pinMode(PIN_RELAY_LIGHT, OUTPUT);
    pinMode(PIN_RELAY_FAN, OUTPUT);

    digitalWrite(PIN_RELAY_LIGHT, LOW);
    digitalWrite(PIN_RELAY_FAN, LOW);

    dhtSensor.begin();
    Wire.begin(21, 22);

    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println(F("[Hardware] BH1750 OK"));
    }
    if (ina219.begin()) {
        Serial.println(F("[Hardware] INA219 OK"));
    }

    setupWiFi();
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    lastEnergyCalcTime = millis();
}

void loop() {
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();

    updateOccupancyState();
    calculateEnergyUsage();

    if (millis() - lastTelemetryTime >= 3000) {
        lastTelemetryTime = millis();
        float temp = 0.0, hum = 0.0, lux = 0.0;
        readSensors(temp, hum, lux);

        if (isAutoMode) {
            processHysteresisControl(temp, lux);
        } else {
            applyActuators(lightState, fanState);
        }

        if (mqttClient.connected()) {
            publishTelemetry(temp, hum, lux);
        }
    }
}

void setupWiFi() {
    delay(10);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(F("\n[Wi-Fi] Connected!"));
}

void connectMQTT() {
    static unsigned long lastReconnect = 0;
    if (millis() - lastReconnect > 5000) {
        lastReconnect = millis();
        if (mqttClient.connect(MQTT_CLIENT_ID, TOPIC_STATUS, 1, true, "offline")) {
            mqttClient.publish(TOPIC_STATUS, "online", true);
            mqttClient.subscribe(TOPIC_COMMAND);
            Serial.println(F("[MQTT] Connected!"));
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    if (deserializeJson(doc, payload, length) == DeserializationError::Ok) {
        if (doc.containsKey("mode")) {
            isAutoMode = (doc["mode"].as<String>() == "auto");
        }
        if (!isAutoMode) {
            if (doc.containsKey("light")) lightState = doc["light"].as<bool>();
            if (doc.containsKey("fan"))   fanState   = doc["fan"].as<bool>();
        }
    }
}

void readSensors(float &temp, float &hum, float &lux) {
    temp = dhtSensor.readTemperature();
    hum  = dhtSensor.readHumidity();
    if (isnan(temp) || isnan(hum)) { temp = 0.0; hum = 0.0; }
    lux = lightMeter.readLightLevel();
    if (lux < 0) lux = 0.0;
}

void updateOccupancyState() {
    if (digitalRead(PIN_PIR) == HIGH || digitalRead(PIN_RADAR) == HIGH) {
        lastMotionTime = millis();
        isOccupied = true;
    } else if (millis() - lastMotionTime > TIME_PIR_TIMEOUT_MS) {
        isOccupied = false;
    }
}

void processHysteresisControl(float temp, float lux) {
    if (!isOccupied) {
        lightState = false;
        fanState   = false;
    } else {
        if (lux < LUX_THRESHOLD_ON) lightState = true;
        else if (lux > (LUX_THRESHOLD_ON + LUX_HYSTERESIS_GAP)) lightState = false;

        if (temp >= TEMP_THRESHOLD_ON) fanState = true;
        else if (temp < (TEMP_THRESHOLD_ON - TEMP_HYSTERESIS_GAP)) fanState = false;
    }
    applyActuators(lightState, fanState);
}

void applyActuators(bool lightTarget, bool fanTarget) {
    digitalWrite(PIN_RELAY_LIGHT, lightTarget ? HIGH : LOW);
    digitalWrite(PIN_RELAY_FAN, fanTarget ? HIGH : LOW);
}

void calculateEnergyUsage() {
    unsigned long now = millis();
    double hours = (now - lastEnergyCalcTime) / 3600000.0;
    lastEnergyCalcTime = now;

    float powerWatts = ina219.getPower_mW() / 1000.0;
    if (powerWatts < 0) powerWatts = 0;

    accumulatedEnergyKwh += (powerWatts * hours) / 1000.0;
}

void publishTelemetry(float temp, float hum, float lux) {
    JsonDocument doc;
    doc["temp"]       = temp;
    doc["hum"]        = hum;
    doc["lux"]        = lux;
    doc["occupied"]   = isOccupied;
    doc["autoMode"]   = isAutoMode;
    doc["light"]      = lightState;
    doc["fan"]        = fanState;
    doc["energyKwh"]  = accumulatedEnergyKwh;

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);
    mqttClient.publish(TOPIC_TELEMETRY, jsonBuffer);
}
