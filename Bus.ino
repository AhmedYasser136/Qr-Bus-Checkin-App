/*****************************************************************
 *  ESP32-CAM → POST JPEG إلى  http://SERVER_IP:3000/scan
 *  يختبر الرد ويطبعه على الـ Serial
 *****************************************************************/
#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

/* ========== بيانات الشبكة ========== */
const char* ssid       = "Your_SSID";
const char* password   = "Your_WIFI_Password";

/* ========== عنوان السيرفر ========== */
const char* serverUrl  = "http://192.168.1.50:3000/scan";

/* ========== ضبط كاميرا AI-Thinker ========== */
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

/* ========== تهيئة الكاميرا ========== */
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // حجم صورة صغير (QVGA) لتقليل الاستهلاك – غيّر إلى VGA أو SXGA إذا احتجت
  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;   // من 0 (أعلى) إلى 63 (أقل)
  config.fb_count     = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Camera init failed");
    while (true) delay(1000);
  }
}

/* ========== الإعداد ========== */
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print('.');
  }
  Serial.println("\n✅ WiFi connected!");

  initCamera();
}

/* ========== رفع الصورة ========== */
void sendPhoto() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Camera capture failed");
    return;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);
  Serial.printf("HTTP %d\n", httpResponseCode);

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.printf("❌ Error POSTing: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
  esp_camera_fb_return(fb);
}

/* ========== الحلقة الرئيسية ========== */
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    sendPhoto();
  } else {
    Serial.println("📶 WiFi disconnected – retrying...");
    WiFi.reconnect();
  }
  delay(10000);   // التقط وارسِل كل 10 ثوانٍ
}
