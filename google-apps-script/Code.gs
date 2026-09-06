/**
 * Motor Monitoring & Predictive Maintenance
 * Google Sheets data logger
 *
 * Expected POST body:
 * {
 *   "current": 1.23,
 *   "temp": 32.5,
 *   "rpm": 1500,
 *   "vibration": 16500,
 *   "status": "NORMAL"
 * }
 */

const SHEET_NAME = "Motor Data";

function doPost(e) {
  try {
    const sheet = getOrCreateSheet_();

    const data = JSON.parse(e.postData.contents);

    if (sheet.getLastRow() === 0) {
      sheet.appendRow([
        "Timestamp",
        "Current (A)",
        "Temperature (°C)",
        "RPM",
        "Vibration",
        "Status"
      ]);
    }

    sheet.appendRow([
      new Date(),
      data.current ?? "",
      data.temp ?? "",
      data.rpm ?? "",
      data.vibration ?? "",
      data.status ?? ""
    ]);

    return ContentService
      .createTextOutput(JSON.stringify({
        success: true
      }))
      .setMimeType(ContentService.MimeType.JSON);

  } catch (error) {
    return ContentService
      .createTextOutput(JSON.stringify({
        success: false,
        error: String(error)
      }))
      .setMimeType(ContentService.MimeType.JSON);
  }
}

function doGet() {
  return ContentService
    .createTextOutput("Motor monitoring logger is running.")
    .setMimeType(ContentService.MimeType.TEXT);
}

function getOrCreateSheet_() {
  const spreadsheet = SpreadsheetApp.getActiveSpreadsheet();

  let sheet = spreadsheet.getSheetByName(SHEET_NAME);

  if (!sheet) {
    sheet = spreadsheet.insertSheet(SHEET_NAME);
  }

  return sheet;
}
