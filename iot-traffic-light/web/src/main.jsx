import React from "react";
import { createRoot } from "react-dom/client";
import mqtt from "mqtt";
import "./style.css";

const MQTT_URL = "wss://broker.emqx.io:8084/mqtt";

// ต้องตรงกับ DEVICE_ID ใน esp8266
const DEVICE_ID = "124-2";

const BASE_TOPIC = `iot-class/traffic-light/${DEVICE_ID}`;
const TOPIC_LDR = `${BASE_TOPIC}/light`;
const TOPIC_TRAFFIC = `${BASE_TOPIC}/traffic`;
const TOPIC_COMMAND = `${BASE_TOPIC}/command`;

function App() {
  const [lightValue, setLightValue] = React.useState("---");
  const [traffic, setTraffic] = React.useState("Off");
  const [lastUpdate, setLastUpdate] = React.useState("-");
  const [connected, setConnected] = React.useState(false);
  const [client, setClient] = React.useState(null);

  React.useEffect(() => {
    const mqttClient = mqtt.connect(MQTT_URL, {
      clientId: `web-${DEVICE_ID}-${Math.random().toString(16).slice(2)}`,
      clean: true,
      reconnectPeriod: 2000,
    });

    mqttClient.on("connect", () => {
      setConnected(true);
      mqttClient.subscribe([TOPIC_LDR, TOPIC_TRAFFIC]);
    });

    mqttClient.on("reconnect", () => setConnected(false));
    mqttClient.on("offline", () => setConnected(false));
    mqttClient.on("close", () => setConnected(false));

    mqttClient.on("message", (topic, payload) => {
      const value = payload.toString();

      if (topic === TOPIC_LDR) {
        setLightValue(value);
        updateTime();
      }

      if (topic === TOPIC_TRAFFIC) {
        setTraffic(value);
        updateTime();
      }
    });

    setClient(mqttClient);

    return () => mqttClient.end();
  }, []);

  function updateTime() {
    const now = new Date();
    setLastUpdate(now.toLocaleString("th-TH"));
  }

  function sendColor(color) {
    if (!client || !connected || isOff) return;
    client.publish(TOPIC_COMMAND, color, { qos: 0 });
  }

  const isOff = traffic === "Off";

  return (
    <main className="container">
      <section className="card">
        <div className="top">
          <div>
            <h1>IoT Traffic Light</h1>
            <p>MQTT + esp8266 + React</p>
          </div>

          <div className={connected ? "status online" : "status offline"}>
            {connected ? "MQTT Connected" : "MQTT Disconnected"}
          </div>
        </div>

        <div className="info-grid">
          <div className="info">
            <span>Light Value (LDR)</span>
            <strong>{lightValue}</strong>
          </div>

          <div className="info">
            <span>Last update time</span>
            <strong>{lastUpdate}</strong>
          </div>
        </div>

        <div className="traffic-box">
          <h2>Traffic Light</h2>

          <div className={`traffic ${isOff ? "traffic-off" : ""}`}>
            <div className={`lamp red ${traffic === "Red" ? "active" : ""}`} />
            <div className={`lamp yellow ${traffic === "Yellow" ? "active" : ""}`} />
            <div className={`lamp green ${traffic === "Green" ? "active" : ""}`} />
          </div>

          <h3>สถานะ: {isOff ? "ปิดอยู่ (Off)" : traffic}</h3>
        </div>

        <div className="controls">
          <button
            className="btn-red"
            disabled={isOff || !connected}
            onClick={() => sendColor("Red")}
          >
            Red
          </button>
          <button
            className="btn-yellow"
            disabled={isOff || !connected}
            onClick={() => sendColor("Yellow")}
          >
            Yellow
          </button>
          <button
            className="btn-green"
            disabled={isOff || !connected}
            onClick={() => sendColor("Green")}
          >
            Green
          </button>
        </div>

        <p className="hint">
          Toggle Switch ที่ต่อบน esp8266 ควบคุมการเปิด/ปิดไฟจราจร — เปิดสวิตช์เพื่อให้ไฟวน
          Red → Yellow → Green อัตโนมัติ (ปุ่มด้านบนสั่งเปลี่ยนสีทันทีได้ระหว่างที่เปิดอยู่),
          ปิดสวิตช์เพื่อดับไฟทั้งหมด
        </p>
      </section>
    </main>
  );
}

createRoot(document.getElementById("root")).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);
