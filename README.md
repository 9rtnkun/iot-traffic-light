# IoT Traffic Light - Hackathon 2 (ควบคุมและ Monitor ไฟจราจรผ่าน MQTT)

โปรเจกต์: ระบบ IoT ควบคุมไฟจราจรและ Monitor ผ่าน MQTT

## Requirement ที่ต้องส่ง (6 ข้อ) และวิธีที่โค้ดนี้ตอบโจทย์
 
| # | Requirement | ทำอย่างไร |
|---|---|---|
| 1 | เชื่อมต่อ MQTT กับ Broker ที่กำหนด | ESP32 เชื่อมต่อผ่าน `PubSubClient` (TCP 1883), Web เชื่อมต่อผ่าน `mqtt.js` (WSS 8084) ไปที่ broker.emqx.io ตัวเดียวกัน |
| 2 | แสดงค่า LDR แบบ Realtime อัปเดตทุก 5 วิ | ESP32 อ่าน `analogRead(PIN_LDR)` แล้ว publish ไป topic `.../light` ทุก 5000 ms |
| 3 | แสดงสถานะไฟจราจร (Red/Yellow/Green) บนเว็บ | ESP32 publish สถานะไปยัง topic `.../traffic`, Web subscribe แล้วแสดงผล |
| 4 | Toggle เปิด/ปิดไฟจราจรผ่าน Button Switch ที่ต่อบน ESP | **ON** = ไฟวนอัตโนมัติ Red → Yellow → Green → Red ตามเวลา, **OFF** = ดับไฟทั้งหมด (ดูรายละเอียดด้านล่าง) |
| 5 | แสดง Animation สถานะไฟจราจรบนเว็บ | ดวงไฟที่ Active จะมี glow เต้น (pulsing) ด้วย CSS `@keyframes`, และ dim ลงทั้งชุดเมื่อสถานะเป็น Off |
| 6 | แสดง Datetime ที่ update ข้อมูลล่าสุด | ทุกครั้งที่ได้รับ MQTT message จะอัปเดตเวลาปัจจุบันด้วย `toLocaleString("th-TH")` |

## พฤติกรรม Toggle Switch (สำคัญ)

ปุ่มที่ต่อกับ ESP32 คือ **Switch แบบ 2 ตำแหน่ง (ON/OFF)** ไม่ใช่ปุ่มกดชั่วขณะ (momentary push button):

- **ON** (สวิตช์ต่อกับ GND, อ่านค่าได้ LOW): ไฟจราจรเริ่มวนใหม่จาก Red และวนอัตโนมัติ
  Red (4 วิ) → Yellow (1 วิ) → Green (3 วิ) → Red → ...
  ปรับเวลาได้ที่ตัวแปร `RED_DURATION`, `YELLOW_DURATION`, `GREEN_DURATION` ใน `main.cpp`
- **OFF** (สวิตช์เปิดวงจร, อ่านค่าได้ HIGH): ไฟทั้ง 3 สีดับหมด และหยุดวน

สถานะนี้ถูก publish ไปที่ topic `.../traffic` เป็นค่า `Red` / `Yellow` / `Green` / `Off` ให้ Web แสดงผลตรงกันเสมอ

> ปุ่มสั่งสีบนหน้า Web ("Red"/"Yellow"/"Green") ใช้ได้เมื่อสวิตช์เปิด (ON) เท่านั้น — กดแล้ว ESP32 จะเปลี่ยนไปสีนั้นทันทีและวนต่อจากสีนั้น ถ้าสวิตช์ OFF ปุ่มจะถูก disable และ ESP32 จะไม่รับคำสั่งใดๆ (เพราะไฟดับอยู่)

## MQTT Broker

ตัวอย่างนี้ใช้ EMQX Public Broker:

- ESP32 (TCP): broker.emqx.io:1883
- Web (WSS): wss://broker.emqx.io:8084/mqtt

พอร์ตอื่นของ broker.emqx.io ที่มีให้เผื่อใช้: 8883 (TLS), 8083 (WebSocket ไม่เข้ารหัส)

**ถ้าอาจารย์กำหนด broker อื่น ให้เปลี่ยนค่า `MQTT_BROKER`/`MQTT_PORT` ใน `main.cpp` และ `MQTT_URL` ใน `main.jsx` ให้ตรงกัน**

สำหรับงานจริง/งานส่ง ให้เปลี่ยน `DEVICE_ID` เป็นรหัสของตัวเอง เพื่อไม่ให้ชนกับเพื่อน (ต้องเปลี่ยนทั้งใน `main.cpp` และ `main.jsx` ให้ตรงกัน)

## 1. ESP32

เปิดโฟลเดอร์ `esp32` ด้วย VS Code + PlatformIO

แก้ใน `src/main.cpp`:

```cpp
const char* WIFI_SSID = "กำหนดเอง";
const char* WIFI_PASSWORD = "กำหนดเอง";
const char* DEVICE_ID = "กำหนดเอง";
```

### Pin ที่ใช้ในตัวอย่าง

- LDR -> GPIO 34
- Toggle Switch (เปิด/ปิดไฟจราจร) -> GPIO 27 (ต่อขาอีกด้านเข้า GND)
- Red LED -> GPIO 25
- Yellow LED -> GPIO 26
- Green LED -> GPIO 33

ถ้าชุดทดลองของอาจารย์ใช้ขาคนละเบอร์ ให้แก้ในโค้ด

## 2. Web React

ติดตั้ง Node.js ก่อน จากนั้นเปิด Terminal ในโฟลเดอร์ `web`


```bash
cd web 
npm install
npm run dev
```

จากนั้นเปิด URL ที่ Vite แสดง เช่น:

```text
http://localhost:1111
```

## MQTT Topics

ตัวอย่างเมื่อ DEVICE_ID = student-01

```text
iot-class/traffic-light/student-01/light     (ESP32 -> Web : ค่า LDR ตัวเลข)
iot-class/traffic-light/student-01/traffic   (ESP32 -> Web : Red / Yellow / Green / Off)
iot-class/traffic-light/student-01/command   (Web -> ESP32 : Red / Yellow / Green, มีผลเมื่อสวิตช์ ON เท่านั้น)
```

## หมายเหตุ

EMQX Public Broker เป็น broker สาธารณะ เหมาะสำหรับทดลอง/ทำงานในห้องเรียน แต่ไม่ควรใช้ส่งข้อมูลสำคัญหรือข้อมูลส่วนตัว
