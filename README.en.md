# IoT Traffic Light - Hackathon 2 (Control and Monitor a Traffic Light over MQTT)

Project: an IoT system that controls and monitors a traffic light over MQTT.

## Required Features (6 items) and How This Code Meets Them

| # | Requirement | How it is done |
|---|---|---|
| 1 | Connect to the specified MQTT broker | The ESP32 connects through `PubSubClient` (TCP 1883). The web app connects through `mqtt.js` (WSS 8084). Both use the same broker, broker.emqx.io |
| 2 | Show the LDR value in real time, updated every 5 seconds | The ESP32 reads `analogRead(PIN_LDR)` and publishes it to the `.../light` topic every 5000 ms |
| 3 | Show the traffic light status (Red/Yellow/Green) on the web | The ESP32 publishes the status to the `.../traffic` topic. The web app subscribes and displays it |
| 4 | Toggle the traffic light on/off with a button switch on the ESP | **ON** = the light cycles automatically Red → Yellow → Green → Red on a timer. **OFF** = all lights off (see details below) |
| 5 | Show an animation of the traffic light status on the web | The active lamp has a pulsing glow built with CSS `@keyframes`, and the whole set dims when the status is Off |
| 6 | Show the date and time of the latest update | Every time an MQTT message arrives, the current time is updated with `toLocaleString("th-TH")` |

## Toggle Switch Behavior (Important)

The switch connected to the ESP32 is a **2-position (ON/OFF) switch**, not a momentary push button:

- **ON** (switch connected to GND, reads LOW): the traffic light restarts from Red and cycles automatically.
  Red (4 s) → Yellow (1 s) → Green (3 s) → Red → ...
  The timing can be adjusted with the variables `RED_DURATION`, `YELLOW_DURATION`, and `GREEN_DURATION` in `main.cpp`.
- **OFF** (switch open circuit, reads HIGH): all 3 lights turn off and the cycle stops.

The state is published to the `.../traffic` topic as `Red` / `Yellow` / `Green` / `Off`, so the web app always shows the same state.

> The color buttons on the web page ("Red"/"Yellow"/"Green") only work when the switch is ON. When pressed, the ESP32 changes to that color immediately and continues the cycle from there. When the switch is OFF, the buttons are disabled and the ESP32 ignores all commands (because the lights are off).

## MQTT Broker

This example uses the EMQX Public Broker:

- ESP32 (TCP): broker.emqx.io:1883
- Web (WSS): wss://broker.emqx.io:8084/mqtt

Other ports available on broker.emqx.io: 8883 (TLS), 8083 (WebSocket, unencrypted)

**If your instructor specifies a different broker, change `MQTT_BROKER`/`MQTT_PORT` in `main.cpp` and `MQTT_URL` in `main.jsx` so they match.**

For real use or submission, change `DEVICE_ID` to your own ID so it does not clash with your classmates. It must be changed in both `main.cpp` and `main.jsx` and must match.

## 1. ESP32

Open the `esp32` folder with VS Code + PlatformIO.

Edit `src/main.cpp`:

```cpp
const char* WIFI_SSID = "your-wifi-name";
const char* WIFI_PASSWORD = "your-wifi-password";
const char* DEVICE_ID = "your-device-id";
```

### Pins used in this example

- LDR -> GPIO 34
- Toggle Switch (traffic light on/off) -> GPIO 27 (connect the other leg to GND)
- Red LED -> GPIO 25
- Yellow LED -> GPIO 26
- Green LED -> GPIO 33

If your lab kit uses different pins, change them in the code.

## 2. Web (React)

Install Node.js first, then open a terminal in the `web` folder.

```bash
cd web
npm install
npm run dev
```

Then open the URL that Vite shows, for example:

```text
http://localhost:1111
```

## MQTT Topics

Example when DEVICE_ID = student-01

```text
iot-class/traffic-light/student-01/light     (ESP32 -> Web : LDR value, numeric)
iot-class/traffic-light/student-01/traffic   (ESP32 -> Web : Red / Yellow / Green / Off)
iot-class/traffic-light/student-01/command   (Web -> ESP32 : Red / Yellow / Green, only takes effect when the switch is ON)
```

## Notes

The EMQX Public Broker is a public broker. It is suitable for experiments and classroom work, but should not be used to send important or personal data.

## License

Released under the [MIT License](LICENSE).
