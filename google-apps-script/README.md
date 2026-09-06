# Google Sheets Logger

The ESP32 sends a JSON POST request to a deployed Google Apps Script Web App.

Expected JSON fields:

```json
{
  "current": 1.23,
  "temp": 32.5,
  "rpm": 1500,
  "vibration": 16500,
  "status": "NORMAL"
}
```

## Setup

1. Create a Google Sheet.
2. Open **Extensions → Apps Script**.
3. Copy `Code.gs` into the Apps Script editor.
4. Set the sheet name if required.
5. Deploy as a Web App.
6. Use the Web App URL in `firmware/secrets.h`.
7. Test the endpoint before connecting the ESP32.

The exact Google Apps Script implementation was not included in the original project report, so the supplied `Code.gs` is a compatible reconstruction based on the JSON payload used by the firmware.
