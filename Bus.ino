#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32QRCodeReader.h>

//////////////////////////////
// 1) بيانات Wi-Fi والسيرفر
//////////////////////////////
const char *ssid = "Your_SSID";
const char *password = "Your_WIFI_PASS";
const char *serverUrl = "http://192.168.1.50:3000/scan"; // ← عدِّل IP السيرفر

//////////////////////////////
// 2) أرجل الـ LEDs
//////////////////////////////
#define LED_GREEN_PIN 12 // أخرج أخضر
#define LED_RED_PIN 14   // أخرج أحمر

//////////////////////////////
// 3) تهيئة قارئ الـ QR
//////////////////////////////
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

//////////////////////////////
// 4) إرسال الـ ID للسيرفر
//////////////////////////////
void postID(const String &id)
{
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  String body = "{\"id\":\"" + id + "\"}";

  int code = http.POST(body);
  String payload = http.getString();

  Serial.printf("[HTTP %d] %s\n", code, payload.c_str());

  // إضاءة الـ LEDs بناءً على الاستجابة
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
  { // خطأ/لا رد
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
  }
  http.end();
}

//////////////////////////////
// 5) مهمة قراءة الـ QR
//////////////////////////////
void qrTask(void *pv)
{
  QRCodeData qr;
  while (true)
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
    vTaskDelay(80 / portTICK_PERIOD_MS); // ~12 فريم/ث
  }
}

//////////////////////////////
// 6) الإعداد
//////////////////////////////
void setup()
{
  Serial.begin(115200);

  // LEDs
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);

  // Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }
  Serial.println(" ✅ Connected");

  // قارئ QR
  reader.setup();
  reader.beginOnCore(1);        // استخدام النواة 1 للمهمة الثقيلة
  xTaskCreate(qrTask, "qrTask", // المهمة
              4096, nullptr, 4, nullptr);
}

void loop()
{
  delay(200);
}
