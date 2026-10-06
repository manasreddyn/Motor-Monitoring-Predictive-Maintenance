# Motor Monitoring & Predictive Maintenance System

An ESP32-S3 based motor health monitoring and protection system that measures **current, temperature, vibration, and RPM** in real time.

The system combines local monitoring with IoT/cloud connectivity:

- ESP32-S3 sensor acquisition and processing
- ACS712 current sensing
- LM35 temperature sensing
- MPU6050 vibration sensing
- Hall-effect RPM measurement
- Threshold-based fault detection
- Relay-based motor shutdown
- 16x2 I2C LCD display
- Green/red status LEDs
- Blynk IoT dashboard
- Google Sheets historical logging

## System Architecture

[![Architecture diagram of manasreddyn/motor-monitoring-predictive-maintenance](https://gitdiagram.com/manasreddyn/motor-monitoring-predictive-maintenance/diagram.png)](https://gitdiagram.com/manasreddyn/motor-monitoring-predictive-maintenance?utm_source=readme&utm_medium=picture)
```

## Hardware

| Component | Function |
|---|---|
| ESP32-S3 | Main controller and Wi-Fi connectivity |
| ACS712 | Motor current measurement |
| LM35 | Motor temperature measurement |
| MPU6050 | Vibration measurement |
| Hall-effect sensor | RPM measurement |
| Relay module | Motor ON/OFF protection |
| 16x2 I2C LCD | Local display |
| Green LED | Normal operation |
| Red LED | Fault indication |
| DC geared motor | Monitored load |

## Pin Mapping

| Component | Signal | ESP32-S3 |
|---|---|---:|
| ACS712 | OUT | GPIO 4 |
| LM35 | OUT | GPIO 1 |
| MPU6050 | SDA | GPIO 8 |
| MPU6050 | SCL | GPIO 9 |
| Hall sensor | DO | GPIO 10 |
| Relay | IN | GPIO 21 |
| Green LED | Anode/control | GPIO 2 |
| Red LED | Anode/control | GPIO 3 |
| I2C LCD | SDA | GPIO 8 |
| I2C LCD | SCL | GPIO 9 |

The MPU6050 and LCD share the same I2C bus.

## Fault Detection

The prototype uses the following thresholds:

| Parameter | Fault condition |
|---|---|
| Temperature | > 40 °C |
| Current | > 10 A |
| Vibration | > 1.2 × calibrated baseline |

When a fault is detected, the firmware activates the relay output and red LED while turning off the green LED.

## Data Processing

### Current

The ACS712 reading is calibrated at startup to determine the zero-current offset. A 20-sample moving average is then applied to reduce noise.

### Temperature

The LM35 output is converted using its 10 mV/°C relationship.

### Vibration

The MPU6050 acceleration magnitude is calculated as:

```text
sqrt(ax² + ay² + az²)
```

A baseline is established during startup calibration.

### RPM

The Hall sensor generates pulses. Pulses are counted over one second and converted to RPM.

## Cloud Logging

The firmware sends JSON data containing:

```json
{
  "current": 0.0,
  "temp": 0.0,
  "rpm": 0,
  "vibration": 0,
  "status": "NORMAL"
}
```
The Google Apps Script endpoint appends these values to a Google Sheet.

## Firmware Setup

1. Install Arduino IDE.
2. Select the appropriate ESP32-S3 board.
3. Install the required libraries:
   - Blynk
   - LiquidCrystal_I2C
   - I2Cdev
   - MPU6050
4. Add your own Blynk and Wi-Fi credentials.
5. Add your Google Apps Script Web App URL.
6. Upload `firmware/motor_monitoring.ino`.

## Safety

This prototype is intended for low-voltage laboratory/demo use. Follow appropriate electrical isolation, overcurrent protection, grounding, mechanical guarding, and wiring practices when testing a motor.

## Future Improvements

- Machine-learning based fault prediction
- Frequency-domain vibration analysis
- Multi-motor monitoring
- MQTT / Modbus / OPC UA integration
- Mobile notifications
- Grafana / Power BI dashboards
- Automated cooling or speed control
- Improved sensor calibration and filtering
