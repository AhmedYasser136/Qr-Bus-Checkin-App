# 📡 QR Bus Scanner – API Specification 

The device scans a QR code and sends the `id` to this API to verify if a passenger is allowed to board the bus.

---

## 🔌 Endpoint

### `POST /scan`

- **Protocol:** HTTP
- **Method:** POST
- **Content-Type:** `application/json`

### ✅ Request Body

```json
{
  "id": "cdd948fa-396e-4226-82cd-591a9d9cb157"
}
```

---

## 📥 Response Format

### ✅ Access Granted

```json
{
  "status": "granted",
  "id": "uuid",
  "name": "Client Name",
  "phone": "010...",
  "travel_date": "YYYY-MM-DD"
}
```

### ❌ Access Denied

```json
{
  "status": "denied",
  "reason": "Invalid travel date",
  "id": "uuid",
  "name": "Client Name"
}
```

### ⚠️ Error

```json
{
  "status": "error",
  "error": "Some error message"
}
```

---

## ✅ Behavior Summary

| Condition              | API Response  | Device Behavior      |
|------------------------|---------------|-----------------------|
| Valid QR & travel date | `granted`     | ✅ Green LED ON       |
| Invalid or expired QR  | `denied`      | ❌ Red LED ON         |
| Connection/server error| `error`       | All LEDs OFF          |

---

