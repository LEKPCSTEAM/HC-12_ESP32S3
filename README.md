## 📡 ESP32S3 + HC-12 433MHz Communication System

ระบบนี้ออกแบบมาเพื่อให้ **ESP32S3** สามารถรับข้อมูลแบบ **ไร้สายผ่านโมดูล HC-12 (433 MHz)** และสั่งการอุปกรณ์เช่น LED และ buzzer โดยใช้โปรโตคอลที่กำหนดเอง และสื่อสารผ่าน serial

---

<img src="docs/esp32s3mini.png" width="200" height="200" />
<img src="docs/hc-12.png" width="200" height="200" />

### 🧠 Components Used

- **ESP32S3 Mini**
- **HC-12 (SI4463) Wireless Serial Module @ 433MHz**
- **WS2812 LED x1**
- **Active Buzzer**
- **Python (sender)** + `pyserial`

---

## ⚙️ System Overview

| ฝั่ง Python (`PacketSender433Mhz`) | ฝั่ง ESP32 (`receiveData`, `handlePacket`) |
| --- | --- |
| สร้าง packet | รับ packet ผ่าน UART จาก HC-12 |
| ใส่ Header / Type / Payload / Checksum | ตรวจสอบความถูกต้อง (header, checksum, end byte) |
| คิวข้อมูล + delay ส่ง | สั่งงาน LED / buzzer จากข้อมูลที่รับ |

---

## 🔐 Custom Serial Packet Protocol

### ✅ Packet Format

```
[HEADER][TYPE][LENGTH][ID (8 bytes)][PAYLOAD][CHECKSUM][END]
```

| Field | Size | Description |
| --- | --- | --- |
| `HEADER` | 1 byte | Always `0xAA` |
| `TYPE` | 1 byte | Message type (ดูด้านล่าง) |
| `LENGTH` | 1 byte | Total length of `[ID + PAYLOAD]` |
| `ID` | 8 bytes | Device ID (เช่น `"DEVICE01"`) |
| `PAYLOAD` | N bytes | ข้อมูลตามประเภท เช่น `"red"` |
| `CHECKSUM` | 1 byte | XOR ของ header, type, length, sum(payload) |
| `END` | 1 byte | Always `0x55` |

---

### 📥 Example Packet (Python → ESP32)

ส่งคำสั่ง `"red"` ไปยังอุปกรณ์ `DEVICE01`:

```python
MessageType = COLOR (0x01)
ID = "DEVICE01"
Payload = "red"
```

- Header: `0xAA`
- Type: `0x01`
- Length: `8 (ID) + 3 (payload) = 11`
- ID: `44 45 56 49 43 45 30 31` (`DEVICE01`)
- Payload: `72 65 64` (`red`)
- Checksum: XOR ของ header, type, length และ sum(payload)
- End: `0x55`

---

### 📦 MessageType Enum

| Name | Value | Description |
| --- | --- | --- |
| `COLOR` | `0x01` | เปลี่ยนสีของ LED |
| `NUMBER` | `0x02` | ส่งค่าตัวเลข |
| `STATUS_MSG` | `0x03` | ส่งสถานะ |
| `TEXT` | `0x04` | ข้อความทั่วไป |

> ⚠️ ใน ESP32 ใช้ชื่อ STATUS_MSG แทน STATUS เพื่อเลี่ยงชนกับระบบ
> 

---

## 💡 Example Behavior (ESP32)

หาก ESP32 ได้รับ packet ที่:

- `ID == DEVICE01`
- `TYPE == COLOR`
- `PAYLOAD == "blue"`

ESP32 จะ:

- เปิด LED เป็นสีน้ำเงิน
- เล่นเพลง Jingle Bells สั้น ๆ
- ปิด LED หลังเล่นจบ

---

## 🐍 Python Sender (main.py)

```bash
python main.py
```

Python script จะ:

- สร้าง packet
- เข้าคิว
- ส่งออกผ่าน Serial (`/dev/tty.usbserial-1230`)
- รองรับ delay และการจัดลำดับอัตโนมัติ

---

## 🚀 ใช้งาน

### เชื่อมต่อสาย:

- ESP32 TX2 (GPIO14) → HC-12 RX
- ESP32 RX2 (GPIO13) → HC-12 TX
- 5V, GND

### ขั้นตอน:

1. Flash โค้ดไปยัง ESP32
2. เปิด Python script (`main.py`)
3. ตรวจสอบ LED และ serial monitor