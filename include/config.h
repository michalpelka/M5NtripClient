#pragma once
#include <Arduino.h>

namespace config {

struct Settings {
    String wifi_ssid;
    String wifi_password;
    String ntrip_host;
    uint16_t ntrip_port;
    String ntrip_mountpoint;
    String ntrip_username;
    String ntrip_password;
    bool valid;
};

// Load configuration from SD card
// Returns true if successful, false otherwise
bool loadFromSD(Settings& settings, const char* filename = "/config.txt");

// Parse a single line from the config file
void parseLine(const String& line, Settings& settings);

// Trim whitespace from string
String trim(const String& str);

}
