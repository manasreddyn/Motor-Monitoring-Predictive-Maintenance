# ESP32-S3 Firmware

`motor_monitoring.ino` contains the embedded firmware for the motor monitoring system.

## Required Libraries

Install these libraries through the Arduino IDE Library Manager or their respective repositories:

- Blynk
- LiquidCrystal_I2C
- I2Cdev
- MPU6050

## Configuration

Create `secrets.h` from `secrets.h.example`:

```text
secrets.h.example → secrets.h
```

Enter your own credentials in `secrets.h`.

The file is intentionally excluded from Git using `.gitignore`.

## Main Functions

- Startup sensor calibration
- ACS712 current measurement
- 20-sample current moving average
- LM35 temperature measurement
- MPU6050 acceleration magnitude
- Hall-effect RPM measurement
- Threshold-based fault detection
- Relay and LED control
- LCD output
- Blynk telemetry
- Periodic Google Sheets logging

## Notes

The relay logic assumes:

```text
RELAY HIGH → Motor OFF / fault
RELAY LOW  → Motor ON / normal
```

If your relay module uses active-low logic, invert the relay control accordingly.
