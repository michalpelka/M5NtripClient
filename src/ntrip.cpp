#include "ntrip.h"
#include <Arduino.h>

namespace ntrip {

    namespace {
        config ntrip_config;
        WiFiClient wifi_client;
        std::string auth_b64;
    }

    config& get_config()
    {
        return ntrip_config;
    }

    WiFiClient& get_wifi_client()
    {
        return wifi_client;
    }
    
    void init_ntrip_client(const std::string& host,
                          uint16_t port,
                          const std::string& mountpoint,
                          const std::string& username,
                          const std::string& password)
    {
        ntrip_config.host = host;
        ntrip_config.port = port;
        ntrip_config.mountpoint = mountpoint;
        ntrip_config.username = username;
        ntrip_config.password = password;
        Serial.println("NTRIP client initialized.");
    }

    std::string _make_auth_string(const std::string& username, const std::string& password)
    {
        const std::string auth = username + ":" + password;
        const unsigned int encoded_size = b64e_size(auth.size()) + 1;
        unsigned char* encoded = new unsigned char[encoded_size];
        b64_encode(reinterpret_cast<const unsigned char*>(auth.c_str()), auth.size(), encoded);
        encoded[encoded_size - 1] = '\0'; // Null-terminate the string
        std::string auth_b64(reinterpret_cast<char*>(encoded));
        delete[] encoded;
        return auth_b64;
    }

    bool connect_ntrip_client()
    {
        if (!wifi_client.connect(ntrip_config.host.c_str(), ntrip_config.port)) {
            Serial.println("Connection to NTRIP server failed.");
            return false;
        }
        Serial.println("Connected to NTRIP server.");
        return true;

    }

    std::string buildNtripRequest() {
        std::string req = "GET /";
        req += get_config().mountpoint;
        req += " HTTP/1.0\r\n";
        req += "User-Agent: NTRIP SimpleClient/1.0\r\n";
        req += "Authorization: Basic " + auth_b64 + "\r\n";
        req += "Connection: close\r\n";
        req += "\r\n";

        return req;
    }

    bool sendGGA(const String& gga)
    {
        String gga_with_crlf = gga + "\r\n"; // NMEA sentences end with CRLF
        const size_t written = wifi_client.write(reinterpret_cast<const uint8_t*>(gga_with_crlf.c_str()), gga_with_crlf.length());

        if (written != gga_with_crlf.length()) {
            Serial.println("Failed to send complete GGA string.");
            return false;
        }

        Serial.println("GGA string sent successfully.");
        return true;
    }

    bool is_ntrip_connected()
    {
        bool connected = wifi_client.connected();
        // Optionally, you can add more checks here to verify the connection state
        return connected;
    }  

    bool authenticate_ntrip_client()
    {
        if (auth_b64.empty())
        {
            auth_b64 = _make_auth_string(ntrip_config.username, ntrip_config.password);
        }

        // Build and send NTRIP authentication request
        const auto request = buildNtripRequest();
        Serial.println("NTRIP Request:");
        Serial.println(request.c_str());  

        const size_t written = wifi_client.write(reinterpret_cast<const uint8_t*>(request.c_str()), request.size());

        if (written != request.size()) {
            Serial.println("Failed to send complete NTRIP request.");
            return false;
        }

        Serial.println("NTRIP request sent successfully.");
        Serial.println("Waiting for NTRIP server response...");
        
        // Implement authentication logic if needed
        return true;
    }

}
