#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32QRCodeReader.h>

/* ‑‑‑‑‑‑‑‑‑‑ 1) إعداد Wi‑Fi والسيرفر ‑‑‑‑‑‑‑‑‑‑ */
const char *ssid       = "ESP";          // ← غيّر اسم الشبكة
const char *password   = "123654789";    // ← غيّر الباسورد
const char *serverUrl  = "http://192.168.1.2:3000/scan";   // ← غيّر IP السيرفر

/* ‑‑‑‑‑‑‑‑‑‑ 2) مخارج الـ LEDs (اختياري) ‑‑‑‑‑‑‑‑‑‑ */
#define LED_GREEN_PIN 16
#define LED_RED_PIN   4

/* ‑‑‑‑‑‑‑‑‑‑ 3) قارئ QR كما هو بالضبط ‑‑‑‑‑‑‑‑‑‑ */
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

/* دالة إرسال الـ ID إلى السيرفر */
void postID(const String &id) {
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  String body = "{\"id\":\"" + id + "\"}";

  int code    = http.POST(body);
  String resp = http.getString();
  Serial.printf("[HTTP %d] %s\n", code, resp.c_str());

  /* مؤشرات (اختياري) */
  if (resp.indexOf("\"granted\"") != -1) {          // ✅
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_RED_PIN,   LOW);
  } else if (resp.indexOf("\"denied\"") != -1) {    // ❌
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN,   HIGH);
  } else {                                          // خطأ
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN,   LOW);
  }
  http.end();
}

/* ‑‑‑‑‑‑‑‑‑‑ 4) مهمة قراءة QR (كما هي + إضافة postID) ‑‑‑‑‑‑‑‑‑‑ */
void onQrCodeTask(void *pvParameters) {
  struct QRCodeData qrCodeData;

  while (true) {
    if (reader.receiveQrCode(&qrCodeData, 100)) {
      Serial.println("Scanned new QRCode");
      if (qrCodeData.valid) {
        Serial.print("Valid payload: ");
        Serial.println((const char *)qrCodeData.payload);
        postID(String((char *)qrCodeData.payload));        // ← إرسال للسيرفر
      } else {
        Serial.print("Invalid payload: ");
        Serial.println((const char *)qrCodeData.payload);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/* ‑‑‑‑‑‑‑‑‑‑ 5) الإعداد (أُضيف Wi‑Fi فقط) ‑‑‑‑‑‑‑‑‑‑ */
void setup() {
  Serial.begin(115200);
  Serial.println();

  /* LEDs */
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN,   OUTPUT);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN,   LOW);

  /* اتصال Wi‑Fi */
  WiFi.begin(ssid, password);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println(" ✅ Connected");

  /* QR setup كما هو */
  reader.setup();
  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);
  Serial.println("Begin on Core 1");

  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

/* ‑‑‑‑‑‑‑‑‑‑ 6) الحلقة الرئيسية (تأكيد الاتصال) ‑‑‑‑‑‑‑‑‑‑ */
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN,   LOW);
  }
  delay(200);
}
