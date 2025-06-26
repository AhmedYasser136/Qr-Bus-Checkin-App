// === server.js ===

const express = require('express');
const sqlite3 = require('sqlite3').verbose();
const QRCode = require('qrcode');
const { v4: uuidv4 } = require('uuid');
const path = require('path');

const app = express();
const port = 3000;

app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

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

// آخر نتيجة
let lastResult = null;

// === POST /scan { id } ← من ESP32-CAM بعد قراءة QR ===
app.post('/scan', (req, res) => {
  const { id } = req.body;
  if (!id) return res.status(400).json({ status: 'error', error: 'Missing id' });

  db.get("SELECT * FROM clients WHERE id = ?", [id], (err, row) => {
    if (err) {
      console.error(err);
      return res.status(500).json({ status: "error", error: "DB error" });
    }
    if (!row) {
      lastResult = { status: "denied", reason: "Client not found", id };
      return res.status(404).json(lastResult);
    }

    const today = new Date().toISOString().slice(0, 10);
    if (row.travel_date === today) {
      lastResult = { status: "granted", id: row.id, name: row.name, phone: row.phone, travel_date: row.travel_date };
      return res.json(lastResult);
    } else {
      lastResult = { status: "denied", reason: "Invalid travel date", id: row.id, name: row.name };
      return res.status(403).json(lastResult);
    }
  });
});

// === عرض آخر نتيجة فحص ===
app.get('/last-result', (_, res) => {
  res.json(lastResult || { status: 'none' });
  lastResult = null;
});

// === إضافة عميل جديد ===
app.post('/add-client', async (req, res) => {
  const { name, phone, travel_date } = req.body;
  if (!name || !phone || !travel_date) {
    return res.status(400).json({ success: false, message: "Missing fields" });
  }

  const id = uuidv4();
  db.run("INSERT INTO clients (id, name, phone, travel_date) VALUES (?, ?, ?, ?)", [id, name, phone, travel_date], async (err) => {
    if (err) {
      console.error(err);
      return res.status(500).json({ success: false, message: "DB error" });
    }

    const qrImage = await QRCode.toDataURL(id, {
      errorCorrectionLevel: 'H',
      type: 'image/png',
      width: 600,
      margin: 2
    });

    res.json({ success: true, qr: qrImage, id });
  });
});

// === استعراض كل العملاء ===
app.get('/all-clients', (_, res) => {
  db.all("SELECT * FROM clients ORDER BY travel_date DESC", (err, rows) => {
    if (err) return res.status(500).json({ error: "DB error" });
    res.json(rows);
  });
});

// === توليد QR لأي ID ===
app.get('/generate-qr/:id', async (req, res) => {
  const id = req.params.id;
  const qrImage = await QRCode.toDataURL(id, {
    errorCorrectionLevel: 'H',
    type: 'image/png',
    width: 600,
    margin: 2
  });
  res.json({ qr: qrImage });
});

// تشغيل السيرفر
app.listen(port, () => {
  console.log(`🚀 QR Check server running at http://localhost:${port}`);
});
