#include "BLEDevice.h"
#include "BLEUtils.h"
#include "esp_sleep.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"
#include "image.h"


BLEAdvertising *pAdvertising;

uint8_t bleMac[6] = {0x5C, 0xC3, 0x36, 0x8C, 0xBC, 0x7C};
uint8_t bleRaw[31];
boolean rawMoreThan31 = false;
uint8_t bleRaw32[32]; 
const char* ssid = "你的wifi名字";
const char* password = "你的wifi密码";//把手机热点设置成对应的wifi

void setup() {
  Serial.begin(115200);

  Config_Init();
  LCD_Init();
  LCD_SetBacklight(100);
  Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 90, WHITE);
  Paint_SetRotate(90);
  LCD_Clear(BLACK);
  delay(1000);


  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    LCD_Clear(BLACK);
    Paint_DrawString_EN(20, 50, "please set wifi name:liu password:liuxian88", &Font20, BLACK, GREEN);
  }
  
  LCD_Clear(BLACK);
  Paint_DrawString_EN(20, 50, "WiFi connected!", &Font20, BLACK, GREEN);
  delay(1000);
}


void loop() {

  HTTPClient http;
  http.begin("http://服务器ip:3000/rawData");//在这填入服务器api
  
  LCD_Clear(BLACK);
  Paint_DrawString_EN(20, 50, "Sending HTTP GET...", &Font20, BLACK, GREEN);
  delay(500);
  
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);
      LCD_Clear(BLACK);  
      Paint_DrawString_EN(20, 50, "Data received!", &Font20, BLACK, GREEN);
      delay(1000);
      
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        LCD_Clear(BLACK);
        Paint_DrawString_EN(20, 50, "JSON parsing error!", &Font20, BLACK, RED);
        Serial.println(error.f_str());
        return;
      }
      
      String rawDataStr = doc["rawData"];
      int rawDataLen = rawDataStr.length() / 2;
      
      LCD_Clear(BLACK);
      Paint_DrawString_EN(20, 50, "Parsing BLE data...", &Font20, BLACK, GREEN);
      delay(500);
      
      for (int i = 0; i < rawDataLen; i++) {
        String byteString = rawDataStr.substring(i * 2, i * 2 + 2);
        if (i < 31) {
          bleRaw[i] = strtol(byteString.c_str(), NULL, 16);
        } else {
          bleRaw32[i - 31] = strtol(byteString.c_str(), NULL, 16);
        }
      }
      
      if (rawDataLen > 31) {
        rawMoreThan31 = true;
      }
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    LCD_Clear(BLACK);
    Paint_DrawString_EN(20, 50, "HTTP GET failed!", &Font20, BLACK, RED);
    delay(1000);
  }
  
  http.end();

  if (UNIVERSAL_MAC_ADDR_NUM == FOUR_UNIVERSAL_MAC_ADDR) {
    bleMac[5] -= 2;
  } else if (UNIVERSAL_MAC_ADDR_NUM == TWO_UNIVERSAL_MAC_ADDR) {
    bleMac[5] -= 1;
  }
  esp_base_mac_addr_set(bleMac);

  BLEDevice::init("");

  pAdvertising = BLEDevice::getAdvertising();

  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  pAdvertising->setScanResponseData(oScanResponseData);

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  pAdvertising->setAdvertisementData(oAdvertisementData);
  
  LCD_Clear(BLACK);
  Paint_DrawString_EN(20, 50, "Configuring BLE...", &Font20, BLACK, GREEN);
  delay(500);

  esp_err_t errRc = ::esp_ble_gap_config_adv_data_raw(bleRaw, 31);
  if (errRc != ESP_OK) {
    Serial.printf("esp_ble_gap_config_adv_data_raw: %d\n", errRc);
    LCD_Clear(BLACK);
    Paint_DrawString_EN(20, 50, "BLE config error!", &Font20, BLACK, RED);
    delay(1000);
  }

  if (rawMoreThan31) {
    errRc = ::esp_ble_gap_config_scan_rsp_data_raw(bleRaw32, sizeof(bleRaw32)/sizeof(bleRaw32[0]));
    if (errRc != ESP_OK) {
      Serial.printf("esp_ble_gap_config_scan_rsp_data_raw: %d\n", errRc);
    }
  }

  pAdvertising->start();
  LCD_Clear(BLACK);
  Paint_DrawString_EN(20, 50, "BLE advertising...", &Font20, BLACK, GREEN);
  delay(1000);
  
  LCD_Clear(BLACK);
  Paint_DrawString_EN(20, 50, "Execution completed!", &Font20, BLACK, GREEN);
  delay(5000);
  LCD_Clear(BLACK);
  delay(1000);

}
