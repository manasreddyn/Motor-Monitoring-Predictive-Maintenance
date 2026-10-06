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

```text
                 ┌──────────────────┐
                 │     DC Motor     │
                 └────────┬─────────┘
                          │
          ┌───────────────┼────────────────┐
          │               │                │
       Current        Temperature       Vibration
       ACS712             LM35           MPU6050
          │               │                │
          └───────────────┼────────────────┘
                          │
                    ┌─────▼─────┐
                    │  ESP32-S3 │
                    │           │
                    │Calibration|
                    │Filtering  |
                    │Processing |
                    │Fault Logic|
                    │Calibration|
                    │Filtering  |
                    │Processing |
                    │Fault Logic|
                    └─────┬─────┘
                          │
             ┌────────────┼─────────────┐
             │            │             │
          Hall RPM      Relay       LCD + LEDs
             │        Protection     Local Status
             │                          |
             └────────────┬─────────────
                          │
                     Wi-Fi / Internet
                          │
                  ┌───────┴────────┐
                  │                │
                Blynk         Google Sheets
              Dashboard        Data Logging
```

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
