# Hardware Pinout

The pin assignments below follow the original project design.

| Component | Pin | ESP32-S3 GPIO | Notes |
|---|---|---:|---|
| ACS712 | OUT | GPIO 4 | Analog current input |
| LM35 | OUT | GPIO 1 | Analog temperature input |
| MPU6050 | SDA | GPIO 8 | I2C |
| MPU6050 | SCL | GPIO 9 | I2C |
| Hall sensor | DO | GPIO 10 | RPM pulse input |
| Relay module | IN | GPIO 21 | Motor control |
| Green LED | Control | GPIO 2 | Normal status |
| Red LED | Control | GPIO 3 | Fault status |
| LCD | SDA | GPIO 8 | Shared I2C bus |
| LCD | SCL | GPIO 9 | Shared I2C bus |

## I2C Addresses

```text
MPU6050 → 0x68
LCD     → 0x27
```

## Power

Use the sensor/module supply voltages appropriate for the specific hardware boards being used, and ensure the ESP32 GPIO voltage limits are respected.

The original project used a common ground between the modules.
