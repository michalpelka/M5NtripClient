#include "config.h"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5GFX.h>

#define SD_SPI_CS_PIN   4
#define SD_SPI_SCK_PIN  18
#define SD_SPI_MISO_PIN 38
#define SD_SPI_MOSI_PIN 23

namespace config {

String trim(const String& str) {
    int start = 0;
    int end = str.length() - 1;
    
    while (start <= end && isspace(str[start])) {
        start++;
    }
    
    while (end >= start && isspace(str[end])) {
        end--;
    }
    
    if (start > end) {
        return "";
    }
    
    return str.substring(start, end + 1);
}

void parseLine(const String& line, Settings& settings) {
    // Skip empty lines and comments
    String trimmedLine = trim(line);
    if (trimmedLine.length() == 0 || trimmedLine[0] == '#') {
        return;
    }
    
    // Find the equals sign
    int equalsPos = trimmedLine.indexOf('=');
    if (equalsPos == -1) {
        return;
    }
    
    String key = trim(trimmedLine.substring(0, equalsPos));
    String value = trim(trimmedLine.substring(equalsPos + 1));
    
    // Parse based on key
    if (key == "WIFI_SSID") {
        settings.wifi_ssid = value;
    } else if (key == "WIFI_PASSWORD") {
        settings.wifi_password = value;
    } else if (key == "NTRIP_HOST") {
        settings.ntrip_host = value;
    } else if (key == "NTRIP_PORT") {
        settings.ntrip_port = value.toInt();
    } else if (key == "NTRIP_MOUNTPOINT") {
        settings.ntrip_mountpoint = value;
    } else if (key == "NTRIP_USERNAME") {
        settings.ntrip_username = value;
    } else if (key == "NTRIP_PASSWORD") {
        settings.ntrip_password = value;
    }
}

bool loadFromSD(Settings& settings, const char* filename) {
    // Initialize settings as invalid
    settings.valid = false;
    
    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    bool sdInitialized = false;
    for (int attempt = 0; attempt < 3; ++attempt) {
        if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) 
        {
            Serial.println("SD Card initialization failed!");
            delay(100);
            
        }
        else {
            Serial.println("SD Card initialized.");
            sdInitialized = true;
            break;
        }
        
    }
    if (!sdInitialized) {
        Serial.println("Failed to initialize SD card after multiple attempts.");
        return false;
    }
    
    Serial.print("Loading config from SD: ");
    Serial.println(filename);
    
    // Check if file exists
    if (!SD.exists(filename)) {
        Serial.println("Config file not found on SD card");
        return false;
    }
    
    // Open file for reading
    File file = SD.open(filename, FILE_READ);
    if (!file) {
        Serial.println("Failed to open config file");
        return false;
    }
    
    // Read file line by line
    while (file.available()) {
        String line = file.readStringUntil('\n');
        parseLine(line, settings);
    }
    
    file.close();
    
    // Validate that all required settings are present
    if (settings.wifi_ssid.length() == 0 ||
        settings.wifi_password.length() == 0 ||
        settings.ntrip_host.length() == 0 ||
        settings.ntrip_port == 0 ||
        settings.ntrip_mountpoint.length() == 0) {
        Serial.println("Config file missing required settings");
        return false;
    }
    
    settings.valid = true;
    Serial.println("Config loaded successfully from SD card");
    Serial.print("  WiFi SSID: ");
    Serial.println(settings.wifi_ssid);
    Serial.print("  NTRIP Host: ");
    Serial.println(settings.ntrip_host);
    Serial.print("  NTRIP Port: ");
    Serial.println(settings.ntrip_port);
    Serial.print("  NTRIP Mountpoint: ");
    Serial.println(settings.ntrip_mountpoint);
    
    return true;
}

}
