# M5NtripClient

A portable NTRIP (Networked Transport of RTCM via Internet Protocol) client for M5Stack devices that receives RTK correction data from NTRIP casters and forwards it to u-blox GNSS receivers for high-precision positioning.

<img width="512" alt="M5NtripClient in action" src="https://github.com/user-attachments/assets/a1ea2a27-c1ac-4ee5-af3a-5710f57d494f" />

## Overview

This project turns your M5Stack into a standalone NTRIP client that:
- Connects to WiFi networks
- Authenticates with NTRIP casters to receive RTCM correction data
- Forwards RTCM data to u-blox GNSS receivers via serial connection
- Sends GGA NMEA sentences back to the caster for accurate corrections
- Displays real-time status information on the M5Stack screen
- Provides a portable, battery-powered RTK solution

## Features

- **Auto-reconnection**: Automatically reconnects to NTRIP server if connection is lost
- **Real-time Display**: Shows WiFi status, NTRIP connection, data rate, GPS quality
- **GGA Feedback**: Sends NMEA GGA sentences to caster every 5 seconds for optimal corrections
- **Multi-tasking**: Uses FreeRTOS tasks for concurrent operations
- **Health Monitoring**: Detects connection issues and automatically attempts recovery
- **GPS Quality Indicators**: Color-coded display showing fix quality (Invalid/GPS/DGPS/RTK Float/RTK Fixed)
- **Button Controls**: Power off and manual reconnection via physical buttons
- **Data Statistics**: Real-time RTCM data rate and total data received

## Hardware Requirements

### Bill of Materials (BOM)

- **M5Stack Core 1** - ESP32-based development kit with display
- **u-blox ZED-F9R** - Multi-band GNSS receiver with RTK support
- **Optional**: Battery pack for M5Stack (for portable operation)

### Wiring Diagram

Connect the M5Stack to the ZED-F9R as follows:

| M5Stack Pin | ZED-F9R Pin | Description |
|-------------|-------------|-------------|
| 5V          | 5V          | Power supply |
| GND         | GND         | Ground |
| GPIO 16     | TXO2        | Serial receive (UART2 TX from GPS) |
| GPIO 17     | RXD2        | Serial transmit (UART2 RX to GPS) |

## Software Requirements

### Dependencies

- [PlatformIO](https://platformio.org/) - Development platform
- [M5Unified](https://github.com/m5stack/M5Unified) - M5Stack unified library
- Arduino framework for ESP32

### Compatible Platforms

- PlatformIO IDE
- Visual Studio Code with PlatformIO extension

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/michalpelka/M5NtripClient.git
cd M5NtripClient
```

### 2. Configure Secrets

Create a secrets header file with your WiFi and NTRIP credentials:

```bash
cp include/Secrets/secrets.h.template include/Secrets/secrets.h
```

Edit `include/Secrets/secrets.h` with your actual credentials:

```cpp
namespace secrets {
    const char* SSID = "your_wifi_ssid";              // Your WiFi network name
    const char* PASSWORD = "your_wifi_password";      // Your WiFi password
    const char* NTRIP_HOST = "ntrip.example.com";     // NTRIP caster hostname
    const uint16_t NTRIP_PORT = 2101;                 // NTRIP caster port (typically 2101)
    const char* NTRIP_MOUNTPOINT = "MOUNTPOINT";      // Your NTRIP mountpoint
    const char* NTRIP_USERNAME = "username";          // NTRIP username
    const char* NTRIP_PASSWORD = "password";          // NTRIP password
}
```

### 3. Configure u-blox Receiver

You need to configure your u-blox ZED-F9R receiver using u-center software:

#### Required Settings:
- **Accept RTCM corrections on UART2 (RXD2)**
- **Output only GGA NMEA sentences on UART2 (TXD2)**
- **Set UART2 baudrate to 115200**

A pre-configured settings file is provided: `p9r+m5stack.ucf`

#### To apply settings in u-center:
1. Connect ZED-F9R to your computer via USB
2. Open u-center software
3. Load the configuration file: `Tools > Receiver Configuration > Load Configuration` → select `p9r+m5stack.ucf`
4. Transfer to receiver: `Tools > Receiver Configuration > Transfer file > GPS`
5. Save to BBR/Flash: `View > Configuration View > CFG > Save current configuration`

<img width="718" height="530" alt="u-center configuration" src="https://github.com/user-attachments/assets/c13e79e1-3609-4edf-ac70-8589c43be128" />

### 4. Build and Upload

Using PlatformIO CLI:
```bash
pio run --target upload
```

Or use the PlatformIO IDE interface to build and upload.

## Usage

### Starting the Device

1. Power on your M5Stack
2. The device will:
   - Connect to WiFi (displays connection status)
   - Connect to NTRIP caster
   - Authenticate with the caster
   - Begin receiving and forwarding RTCM data

### Display Information

The M5Stack display shows:
- **IP Address**: Current WiFi IP
- **WiFi RSSI**: Signal strength in dBm
- **RTCM Data Rate**: Current data rate in bytes/second
- **Total Data**: Cumulative data received in KB
- **NTRIP Status**: Connected (green) or disconnected (red) with retry count
- **GGA Quality**: GPS fix quality with color coding:
  - 🔴 Red (0): Invalid/No fix
  - 🟡 Yellow (1): GPS fix
  - 🔵 Cyan (2): DGPS fix
  - 🟢 Green (4): RTK Float
  - 🟢 Green-Yellow (5): RTK Fixed
- **GGA Sentence**: Current NMEA GGA sentence from receiver

### Button Controls

- **Button A (Left)**: Hold to power off the device
- **Button B (Middle)**: Hold to force NTRIP reconnection
- **Button C (Right)**: Not currently assigned

### Serial Monitor

Connect to serial monitor at 115200 baud to see debug information:
```bash
pio device monitor
```

## Troubleshooting

### Cannot Connect to WiFi
- Verify SSID and password in `secrets.h`
- Check WiFi signal strength
- Ensure 2.4GHz WiFi network (ESP32 doesn't support 5GHz)

### Cannot Connect to NTRIP Server
- Verify NTRIP host, port, and credentials in `secrets.h`
- Check that mountpoint exists and is accessible
- Verify internet connectivity
- Some NTRIP services require valid subscription

### No RTK Fix
- Ensure ZED-F9R has clear view of sky
- Verify RTCM data is being received (check data rate on display)
- Confirm GGA sentences are being sent to caster
- Check that mountpoint provides corrections for your location
- RTK requires time to converge (typically 30 seconds to several minutes)

### No Data Displayed
- Verify wiring between M5Stack and ZED-F9R
- Check baudrate configuration (115200)
- Ensure ZED-F9R is configured to output GGA on UART2

### Device Restarts/Crashes
- Check power supply (USB or battery)
- Monitor serial output for error messages
- Verify all connections are secure

## Development

### Project Structure
```
M5NtripClient/
├── src/
│   ├── main.cpp        # Main application logic
│   ├── ntrip.cpp       # NTRIP client implementation
│   └── base64.cpp      # Base64 encoding for authentication
├── include/
│   ├── ntrip.h         # NTRIP client header
│   ├── base64.h        # Base64 encoding header
│   └── Secrets/
│       └── secrets.h.template  # Template for credentials
├── platformio.ini      # PlatformIO configuration
└── p9r+m5stack.ucf     # u-blox receiver configuration
```

### Key Components

- **FreeRTOS Tasks**: 
  - `lcdUpdateTask`: Updates display every second
  - `ggaReadTask`: Reads GGA sentences from GPS
  - `healthCheckTask`: Monitors connection health
- **Main Loop**: Handles NTRIP data reception and GGA transmission

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [M5Unified](https://github.com/m5stack/M5Unified) library
- Uses PlatformIO development platform
- Implements NTRIP protocol for RTK corrections (custom implementation following NTRIP v1.0 specification)
- Utilizes [Base64.c](https://github.com/joedf/base64.c) library

## Author

Michał Pełka - [michalpelka](https://github.com/michalpelka)
