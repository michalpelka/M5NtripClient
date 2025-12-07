#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include "Secrets/secrets.h"
#include "ntrip.h"

uint64_t dataCount = 0;

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

SemaphoreHandle_t ggaMutex;
String ggaSentence = "";
SemaphoreHandle_t connectionStateMutex;
bool ntripConnected = false;
int retryCount = 0;


std::vector<String> splitString(const String& str, char delimiter) {
  std::vector<String> tokens;
  String token;
  for (unsigned int i = 0; i < str.length(); i++) {
    if (str[i] == delimiter) {
      tokens.push_back(token);
      token = "";
    } else {
      token += str[i];
    }
  }
  tokens.push_back(token); // Add the last token
  return tokens;
}

int GGAQualityToColor(const char ggaQuality) {
  if (ggaQuality == '0') {
    return TFT_RED; // Invalid
  } 
  if (ggaQuality == '1') {
    return TFT_YELLOW; // GPS fix
  } 
  if (ggaQuality == '2') {
    return TFT_CYAN; // DGPS fix
  } 
  if (ggaQuality == '4') {
    return TFT_GREEN; // RTK Fixed or RTK Float
  } 
  if (ggaQuality == '5') {
    return TFT_GREENYELLOW; // RTK Fixed or RTK Float
  }
  return TFT_WHITE; // Unknown quality
  
}
void lcdUpdateTask(void* parameter) {
  uint64_t previousDataCount = 0;
  bool firstRun = true;
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    uint64_t currentDataCount = dataCount;
    uint64_t dataRate = currentDataCount - previousDataCount;
    previousDataCount = currentDataCount;

    xSemaphoreTake(connectionStateMutex, portMAX_DELAY);
    bool connected = ntripConnected;
    xSemaphoreGive(connectionStateMutex);

    M5.Display.setTextSize(2);
    if (firstRun) {
      // clear screen once on task start to avoid flashing on subsequent updates
      M5.Lcd.fillScreen(TFT_BLACK);
      firstRun = false;
    }

    // Use text background color to overwrite previous lines instead of clearing whole screen
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.printf("IP :%s\n", WiFi.localIP().toString().c_str());
    M5.Lcd.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
    M5.Lcd.printf("RTCMData Rate: %llu B/s\n", dataRate);
    M5.Lcd.printf("Total Data: %llu KBytes\n", currentDataCount/1024);

    if (connected) {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
      M5.Lcd.printf("NTRIP: Connected \n");
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
      M5.Lcd.printf("NTRIP: NOK, r:  %d\n", retryCount);
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    }

    String ggaCopy = "";
    // Copy GGA sentence safely
    xSemaphoreTake(ggaMutex, portMAX_DELAY);
    ggaCopy = ggaSentence;
    xSemaphoreGive(ggaMutex);

    // get quality
    char quality = '0';
    // split nmea by comma
    std::vector<String> nmeaParts = splitString(ggaCopy, ',');
    if (nmeaParts.size() > 6) {
      quality = nmeaParts[6].charAt(0);
    }
    const auto qualityColor = GGAQualityToColor(quality);
    M5.Lcd.setTextColor(qualityColor, TFT_BLACK);
    M5.Lcd.printf("GGA Quality: %c\n\n", quality);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.println(ggaCopy);

    if (M5.BtnA.getState() == m5::Button_Class::button_state_t::state_hold) {
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.print("Goodbye!");
      delay(1000);
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Power.powerOff();
    }
    M5.update();
  }
}

void ggaReadTask(void* parameter) {
  for (;;) {
    if (Serial2.available()) {
      String gga = Serial2.readStringUntil('\n');
      if (gga.startsWith("$GPGGA")) {
        xSemaphoreTake(ggaMutex, portMAX_DELAY);
        ggaSentence = gga;
        xSemaphoreGive(ggaMutex);
      }
    } else {
      vTaskDelay(pdMS_TO_TICKS(10)); // Add a small delay to prevent busy-wait
    }
  }
}

void healthCheckTask(void* parameter) {
  uint64_t previousDataCount = 0;

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 2 seconds
    // Implement health check logic if needed
    uint64_t currentDataCount = dataCount;
    uint64_t dataRate = currentDataCount - previousDataCount;
    previousDataCount = currentDataCount;
    // If no data received in last interval, consider connection lost
    
    if (dataRate == 0 || M5.BtnB.getState() == m5::Button_Class::button_state_t::state_hold) {
      xSemaphoreTake(connectionStateMutex, portMAX_DELAY);
      if (ntripConnected) {        
        ntripConnected = false;
        ntrip::get_wifi_client().stop();
        Serial.println("Health Check: No data received, marking NTRIP as disconnected.");
      }
      xSemaphoreGive(connectionStateMutex); 
    }
    
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial.write("Starting M5NtripClient...\n");
  M5.begin();
  M5.Lcd.print("Booted!");
  // Connect to WiFi
  M5.Lcd.print("\nConnecting to WiFi");
  WiFi.begin(secrets::SSID, secrets::PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
  }
  Serial.write("Connected to WiFi!");
  M5.Lcd.printf("\nConnected to WiFi! %s", secrets::SSID);
  M5.Lcd.print("\nIP Address: ");
  M5.Lcd.print(WiFi.localIP());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  M5.Lcd.print("\nConnecting to NTRIP at ");
  M5.Lcd.print(secrets::NTRIP_HOST);
  M5.Lcd.print(":");
  M5.Lcd.print(secrets::NTRIP_PORT);

  // ntrip client initialization example
  ntrip::init_ntrip_client(secrets::NTRIP_HOST, secrets::NTRIP_PORT, secrets::NTRIP_MOUNTPOINT, secrets::NTRIP_USERNAME, secrets::NTRIP_PASSWORD);
  while (!ntrip::connect_ntrip_client()) {
    delay(2000); // Retry every 2 seconds
    M5.Lcd.print(".");
  }

  M5.Lcd.print("\nNTRIP Connected!");
  ntrip::authenticate_ntrip_client();
  M5.Lcd.print("\nNTRIP Authenticated!");

  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("NTRIP Client Running\n");
  delay(2000);

  // Create LCD update task
  xTaskCreate(
    lcdUpdateTask,        // Task function
    "LCD Update Task",    // Name of the task
    2048,                 // Stack size (in words)
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    NULL                  // Task handle
  );

  // Create GGA read task
  ggaMutex = xSemaphoreCreateMutex();
  xTaskCreate(
    ggaReadTask,          // Task function
    "GGA Read Task",      // Name of the task
    2048,                 // Stack size (in words)      
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    NULL                  // Task handle
  );

  // Create connection health check task
  xTaskCreate(
    healthCheckTask,     // Task function
    "Health Check Task",  // Name of the task
    2048,                 // Stack size (in words)
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    NULL                  // Task handle
  );

  connectionStateMutex = xSemaphoreCreateMutex();

}


void loop() {  
  //raise mutex 
  xSemaphoreTake(connectionStateMutex, portMAX_DELAY);
  ntripConnected = ntrip::is_ntrip_connected();
  xSemaphoreGive(connectionStateMutex);

  if (!ntripConnected) {
    // attempt to reconnect
    Serial.println("NTRIP connection lost. Reconnecting...");
    if (ntrip::connect_ntrip_client()) {
      Serial.println("Reconnected to NTRIP server.");
      retryCount = 0;
      ntrip::authenticate_ntrip_client();
    } else {
      Serial.println("Reconnection to NTRIP server failed.");
      retryCount++;
      delay(200); // wait before retrying
      return;
    }
  }
  if (ntrip::get_wifi_client().available()) {
    uint8_t buffer[128];
    size_t len = ntrip::get_wifi_client().read(buffer, sizeof(buffer));
    // write to Serial for debugging
    Serial2.write(buffer, len);
    dataCount += len; 
  }
  // every 5 sec  report gga to caster
  static uint32_t lastGGAReport = 0;
  if (millis() - lastGGAReport > 5000) {
    lastGGAReport = millis();
    // copy gga sentence safely
    String ggaCopy = "";
    xSemaphoreTake(ggaMutex, portMAX_DELAY);
    ggaCopy = ggaSentence;
    xSemaphoreGive(ggaMutex);
    if (ggaCopy.length() > 0) {
      ntrip::sendGGA(ggaCopy);
    }
  }
}

