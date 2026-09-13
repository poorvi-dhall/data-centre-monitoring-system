# Data Centre Monitoring System

An IoT-based data centre monitoring system that collects environmental and activity data using ESP32 sensors, transmits the data over MQTT, and processes it using Node-RED.

## System Architecture

```text
ESP32 / Wokwi
     ↓
 MQTT (HiveMQ)
     ↓
  Node-RED
     ↓
 Dashboard & Risk Analysis