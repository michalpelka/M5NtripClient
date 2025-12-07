#include <string>
#include <WiFi.h>
#include "base64.h"

namespace ntrip {

    struct config
    {
        std::string host;
        uint16_t port;
        std::string mountpoint;
        std::string username;
        std::string password;
    };

    config& get_config();

    WiFiClient& get_wifi_client();
    
    void init_ntrip_client(const std::string& host,
                          uint16_t port,
                          const std::string& mountpoint,
                          const std::string& username,
                          const std::string& password);

    std::string _make_auth_string(const std::string& username, const std::string& password);

    bool connect_ntrip_client();

    std::string buildNtripRequest();

    bool sendGGA(const String& gga);

    bool is_ntrip_connected();

    bool authenticate_ntrip_client();

}
