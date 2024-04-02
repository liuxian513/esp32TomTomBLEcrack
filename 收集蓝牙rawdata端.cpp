#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#if __has_include("esp_eap_client.h")
#include "esp_eap_client.h"
#else
#include "esp_wpa2.h"
#endif
#include <Wire.h>
#include <esp_task_wdt.h> // 引入看门狗定时器的头文件


#define EAP_IDENTITY "wifi账户"
#define EAP_PASSWORD "wifi密码"
const char *ssid = "wifi名字";
int counter = 0;
int scanTime = 1; // In seconds
unsigned long startMillis;  // 记录程序启动的时间
unsigned long restartInterval = 1800000; // 30分钟以毫秒为单位



BLEScan* pBLEScan;

const int LED_PIN = 2; // 定义ESP32的LED引脚

BLEUUID serviceUUID("填入要扫描设备的serviceUUID");  //全部小写字母
const char* targetDeviceMacAddress = "填入要扫描设备的mac地址";  //全部小写字母


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override {

      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)&&advertisedDevice.getAddress().toString() == targetDeviceMacAddress){ 
        pBLEScan->stop();
        Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

        String rawDataHex = "02010617FF"; 
        std::string rawData = advertisedDevice.getManufacturerData();
        for (auto byte : rawData) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02X", (unsigned char)byte);
            rawDataHex += buf;
        }
        rawDataHex += "03033CFE0C0952544B5F42545F342E3100";

        String jsonString = "{\"rawData\":\"" + rawDataHex + "\"}";

        HTTPClient http;
        http.begin("http://112.74.39.16:3000/data");
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(jsonString);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        http.end();

        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
      }
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  delay(10);
  Serial.println();
  Serial.print("Connecting to network: ");
  Serial.println(ssid);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
#if __has_include("esp_eap_client.h")
  esp_eap_client_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_eap_client_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_eap_client_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_enterprise_enable();
#else
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
#endif
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if (counter >= 60) {
      ESP.restart();
    }
  }
  Serial.println("WiFi connected");
  Serial.println("IP address set: ");
  Serial.println(WiFi.localIP());

  startMillis = millis();  // 初始化启动时间

  esp_task_wdt_init(30, true);// 初始化看门狗定时器，设置超时为30秒
  
  esp_task_wdt_add(NULL);// 将当前任务添加到看门狗监控列表



  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
    unsigned long startTime = millis(); // 记录扫描开始的时间
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    unsigned long scanDuration = millis() - startTime; // 计算扫描持续的时间
    Serial.println("Scan done!");
    if (scanDuration > (2000)) {
        Serial.println("BLE scan timeout, restarting...");
        ESP.restart(); // 如果检测到超时，则重启ESP32
    }
    pBLEScan->clearResults(); // 清除扫描结果以释放内存


if (millis() - startMillis > restartInterval) {
        Serial.println("Restarting after 30 minutes...");
        ESP.restart();
    }


  if (WiFi.status() == WL_CONNECTED) {
    counter = 0;
    Serial.println("Wifi is still connected with IP: ");
    Serial.println(WiFi.localIP());
  } else if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid);
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if (counter >= 60) {
      ESP.restart();
      esp_task_wdt_reset(); // 在网络等待循环中也要不断重置看门狗定时器
    }
  }
  delay(10000);
  esp_task_wdt_reset(); // 也在长时间的delay后重置看门狗定时器
}