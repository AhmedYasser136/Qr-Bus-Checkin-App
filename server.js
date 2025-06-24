// === server.js ===
const express = require('express');
const multer = require('multer');
const sharp = require('sharp');
const jsQR = require('jsqr');
const sqlite3 = require('sqlite3').verbose();
const path = require('path');
const QRCode = require('qrcode');
const { v4: uuidv4 } = require('uuid');

const app = express();
const port = 3000;

// إعداد ملفات الواجهة
app.use(express.static(path.join(__dirname, 'public')));
app.use(express.json());

// قاعدة البيانات
const db = new sqlite3.Database('./clients.db');
db.serialize(() => {
  db.run(`CREATE TABLE IF NOT EXISTS clients (
    id TEXT PRIMARY KEY,
    name TEXT,
    phone TEXT,
    travel_date DATE
  )`);
});

// تخزين مؤقت للنتيجة الأخيرة
let lastResult = null;

// استقبال صور من ESP32-CAM
const storage = multer.memoryStorage();
const upload = multer({ storage: storage });

// مسار استقبال وتحليل الصورة
app.post('/scan', upload.single('image'), async (req, res) => {
  try {
    const buffer = req.file ? req.file.buffer : req.body;
    const raw = await sharp(buffer).raw().toBuffer({ resolveWithObject: true });
    const qrCode = jsQR(new Uint8ClampedArray(raw.data), raw.info.width, raw.info.height);

    if (!qrCode) {
      lastResult = { status: "denied", reason: "QR not found" };
      return res.status(400).json(lastResult);
    }

    const qrData = qrCode.data.trim();
    db.get("SELECT * FROM clients WHERE id = ?", [qrData], (err, row) => {
      if (err || !row) {
        lastResult = { status: "denied", reason: "Client not found" };
        return res.status(404).json(lastResult);
      }

      const today = new Date().toISOString().slice(0, 10);
      if (row.travel_date === today) {
        lastResult = { status: "granted", data: row.id, name: row.name, phone: row.phone, travel_date: row.travel_date };
        return res.json(lastResult);
      } else {
        lastResult = { status: "denied", reason: "Invalid travel date", data: row.id, name: row.name };
        return res.status(403).json(lastResult);
      }
    });
  } catch (err) {
    console.error("Error:", err.message);
    return res.status(500).json({ status: "error", error: err.message });
  }
});

// API تعرض آخر نتيجة
app.get('/last-result', (req, res) => {
  if (lastResult) {
    res.json(lastResult);
  } else {
    res.json({ status: "none" });
  }
});

// API لإضافة عميل وتوليد QR
app.post('/add-client', async (req, res) => {
  const { name, phone, travel_date } = req.body;
  if (!name || !phone || !travel_date) {
    return res.status(400).json({ success: false, message: "Missing fields" });
  }
  const id = uuidv4();
  db.run("INSERT INTO clients (id, name, phone, travel_date) VALUES (?, ?, ?, ?)", [id, name, phone, travel_date], async (err) => {
    if (err) {
      console.error("DB insert error:", err.message);
      return res.status(500).json({ success: false, message: "DB error" });
    }
    const qrImage = await QRCode.toDataURL(id);
    return res.json({ success: true, qr: qrImage, id });
  });
});

// API تعرض جميع العملاء
app.get('/all-clients', (req, res) => {
  db.all("SELECT * FROM clients ORDER BY travel_date DESC", (err, rows) => {
    if (err) return res.status(500).json({ error: "DB error" });
    res.json(rows);
  });
});

// تشغيل السيرفر
app.listen(port, () => {
  console.log(`QR server running at http://localhost:${port}`);
});
