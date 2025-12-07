# M5NtripClient
M5 Ntrip client 

<img width="512" alt="image" src="https://github.com/user-attachments/assets/a1ea2a27-c1ac-4ee5-af3a-5710f57d494f" />

## BOM
- M5 Stack Core 1
- Ublox ZED-F9R
- Optional battery pack for M5 Stack

## Wiring

| Stack  | ZED-F9R |
|--------|---------|
| 5V     | 5V      |
| GND    | GND     |
| 16     | TXO2    |
| 17     | RXD2    |

## Ucenter settings 
You need to configure Ublox reciever:
- accept RTCM on RXD2
- send GGA (only) to TXD2
- set baudrate to 115200
![ZED-F9R config](./p9r+m5stack.ucf)

<img width="718" height="530" alt="image" src="https://github.com/user-attachments/assets/c13e79e1-3609-4edf-ac70-8589c43be128" />

