#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// =====================
// 1) Wi-Fi (ข้อ 1)
// =====================
const char* WIFI_SSID = "***"; // กำหนดชื่อและรหัสผ่าน Wi-Fi ของคุณ
const char* WIFI_PASSWORD = "***"; // กำหนดชื่อและรหัสผ่าน Wi-Fi ของคุณ

// =====================
// 2) MQTT (ข้อ 1)
// =====================
const char* MQTT_BROKER = "broker.emqx.io";
const int   MQTT_PORT = 1883;
const char* DEVICE_ID = "***"; // กำหนดชื่ออุปกรณ์ของคุณ

String baseTopic;
String topicLdr;      // ข้อ 2: ส่งค่า LDR
String topicTraffic;  // ข้อ 3: แสดงสถานะสีปัจจุบัน
String topicCommand;  // ข้อ 5: รับคำสั่งเปิดไฟจาก Web

// =====================
// 3) ขา Pin สำหรับ ESP8266 (NodeMCU)
// =====================
const int PIN_LDR    = A0; 
const int PIN_SWITCH = D5; 
const int PIN_RED    = D1; 
const int PIN_YELLOW = D2; 
const int PIN_GREEN  = D3; 

WiFiClient espClient;
PubSubClient mqtt(espClient);

// ลำดับการเปลี่ยนสีตามข้อ 4: Red (0) -> Yellow (1) -> Green (2) -> Yellow (3)
int toggleIndex = 0; 
String currentLightColor = "Red";

int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// ตัวแปรสำหรับส่งค่าเซนเซอร์ LDR ทุก 5 วินาที (ข้อ 2)
unsigned long lastSensorPublish = 0;
const unsigned long SENSOR_INTERVAL = 5000;

void applyHardwareLights(String color) {
  currentLightColor = color;
  digitalWrite(PIN_RED, (color == "Red") ? HIGH : LOW);
  digitalWrite(PIN_YELLOW, (color == "Yellow") ? HIGH : LOW);
  digitalWrite(PIN_GREEN, (color == "Green") ? HIGH : LOW);
}

void publishTrafficState() {
  mqtt.publish(topicTraffic.c_str(), currentLightColor.c_str(), true);
}

void setTrafficLight(String color) {
  applyHardwareLights(color);
  publishTrafficState();
}

// ฟังก์ชัน Toggle สีไฟเมื่อกดปุ่ม: Red -> Yellow -> Green -> Yellow -> Red (ข้อ 4)
void advanceTrafficLight() {
  toggleIndex = (toggleIndex + 1) % 4;
  
  if (toggleIndex == 0) setTrafficLight("Red");
  else if (toggleIndex == 1) setTrafficLight("Yellow");
  else if (toggleIndex == 2) setTrafficLight("Green");
  else if (toggleIndex == 3) setTrafficLight("Yellow");
}

// ข้อ 5: รับคำสั่งเปิดไฟจากหน้าเว็บ
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == topicCommand) {
    message.trim();
    if (message.equalsIgnoreCase("Red")) {
      toggleIndex = 0;
      setTrafficLight("Red");
    } else if (message.equalsIgnoreCase("Yellow")) {
      toggleIndex = 1;
      setTrafficLight("Yellow");
    } else if (message.equalsIgnoreCase("Green")) {
      toggleIndex = 2;
      setTrafficLight("Green");
    }
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void connectMQTT() {
  while (!mqtt.connected()) {
    String clientId = String(DEVICE_ID) + "-" + String(ESP.getChipId(), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(topicCommand.c_str());
      publishTrafficState(); // ส่งสถานะเริ่มต้น
    } else {
      delay(2000);
    }
  }
}

void publishSensor() {
  int ldrValue = analogRead(PIN_LDR);
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d", ldrValue);
  mqtt.publish(topicLdr.c_str(), buffer, true);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_SWITCH, INPUT_PULLUP);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  // เริ่มต้นที่ Red ตามไดอะแกรม Start -> Red
  applyHardwareLights("Red");

  baseTopic = String("iot-class/traffic-light/") + DEVICE_ID;
  topicLdr = baseTopic + "/light";
  topicTraffic = baseTopic + "/traffic";
  topicCommand = baseTopic + "/command";

  connectWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  // ข้อ 4: เปลี่ยนสีไฟเฉพาะเมื่อกดปุ่ม Switch ที่บอร์ดเท่านั้น
  int reading = digitalRead(PIN_SWITCH);
  if (reading == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > 200) {
    lastDebounceTime = millis();
    advanceTrafficLight();
  }
  lastButtonState = reading;

  // ข้อ 2: ส่งค่าเซนเซอร์แสง (LDR) ขึ้น Web ทุกๆ 5 วินาที
  if (millis() - lastSensorPublish >= SENSOR_INTERVAL) {
    lastSensorPublish = millis();
    publishSensor();
  }
}
