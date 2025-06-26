# QR Bus Check-In Device 🚌📷

جهاز يعمل بـ ESP32-CAM لقراءة QR والتحقق من صلاحية الركوب على الباص عن طريق API.

---

## 🧩 مكونات المشروع

- ✅ `server.js` → سيرفر Node.js للتأكد من الحجز
- ✅ `clients.db` → قاعدة SQLite تحتوي على جدول العملاء
- ✅ `esp32_cam.ino` → كود ESP32-CAM (يفك QR ويبعته للسيرفر)

---

## 🔌 تجهيز السيرفر

### 1. المتطلبات:
- Node.js
- SQLite (ملف قاعدة بيانات اسمه `clients.db` يحتوي على جدول `clients`):

```sql
CREATE TABLE clients (
  id TEXT PRIMARY KEY,
  name TEXT,
  phone TEXT,
  travel_date TEXT
);
