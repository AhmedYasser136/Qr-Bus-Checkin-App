#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32QRCodeReader.h>

/* 1) شبكة Wi-Fi والسيرفر */
const char *ssid = "Your_SSID";
const char *password = "Your_WIFI_PASS";
const char *serverUrl = "http://192.168.1.50:3000/scan"; // ← عدِّل IP

/* 2) مخارج الـ LEDs */
#define LED_GREEN_PIN 12
#define LED_RED_PIN 14

/* 3) قارئ QR */
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

/* 4) إرسال الـ ID */
void postID(const String &id)
{
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  String body = "{\"id\":\"" + id + "\"}";

  int code = http.POST(body);
  String payload = http.getString();
  Serial.printf("[HTTP %d] %s\n", code, payload.c_str());

  if (payload.indexOf("\"granted\"") != -1)
  { // ✅
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_RED_PIN, LOW);
  }
  else if (payload.indexOf("\"denied\"") != -1)
  { // ❌
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, HIGH);
  }
  else
  { // خطأ
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
  }
  http.end();
}

/* 5) مهمة قراءة QR */
void qrTask(void *)
{
  QRCodeData qr;
  for (;;)
  {
    if (reader.receiveQrCode(&qr, 100))
    {
      if (qr.valid)
      {
        Serial.printf("QR OK: %s\n", (char *)qr.payload);
        postID(String((char *)qr.payload));
      }
      else
      {
        Serial.println("QR غير صالح");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(80)); // ~12FPS
  }
}

/* 6) الإعداد */
void setup()
{
  Serial.begin(115200);

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }
  Serial.println(" ✅ Connected");

  reader.setup();
  reader.beginOnCore(1);
  xTaskCreate(qrTask, "qrTask", 4096, nullptr, 4, nullptr);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.reconnect();
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
  }
  delay(200);
}
