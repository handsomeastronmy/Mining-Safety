function doPost(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  var data = JSON.parse(e.postData.contents); // Parse incoming JSON

  // Ensure the sheet has a header row if it doesn't exist
  if (sheet.getLastRow() === 0) {
    sheet.appendRow([
      "Sheet Time",           // Current time of data logging
      "Timestamp (MM:SS)",    // ESP32 timestamp in minutes and seconds
      "CO (ppm)",             // CO level from MQ sensor
      "Vibration Intensity",  // Vibration intensity
      "Humidity (%)",         // Humidity from DHT sensor
      "X Angle (°)",          // Roll angle
    ]);
  }

  // Extract ESP32 timestamp and format it to MM:SS
  var esp32Time = new Date(data.timestamp || new Date()); // Fallback to current time
  var minutes = esp32Time.getMinutes();
  var seconds = esp32Time.getSeconds();
  var formattedTimestamp = `${minutes}:${seconds}`;

  // Append new sensor data to the Google Sheet
  sheet.appendRow([
    new Date(),                  // Current time in sheet
    formattedTimestamp,          // ESP32 timestamp in MM:SS
    data.co || "N/A",            // CO level
    data["vibration Intensity"] || "N/A", // Vibration intensity
    data.Humidity || "N/A",      // Humidity
    data.Roll || "N/A",          // X angle (roll)
  ]);

  // Confirm data receipt
  return ContentService.createTextOutput("Data received successfully");
}
