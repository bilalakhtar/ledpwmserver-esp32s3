# XIAO ESP32S3 Wi-Fi LED Brightness Controller

Control the **built-in user LED** on a **Seeed Studio XIAO ESP32S3** from a browser using a Wi-Fi **Access Point (AP)**. The ESP32S3 creates its own Wi-Fi network, hosts a responsive webpage with a **0–100% brightness slider** and **ON/OFF buttons**, and prints diagnostic information to the **Arduino IDE Serial Monitor**.

No external Wi-Fi router or internet connection is required.

## Features

- Standalone Wi-Fi Access Point (`XIAO-LED-Control`)
- Browser-based LED brightness slider (0–100%)
- ON/OFF buttons
- PWM control of the built-in, **active-low** user LED on **GPIO21**
- Serial diagnostics for AP startup, brightness changes, PWM duty cycle, HTTP requests, and connected client count
- Periodic status reports every 5 seconds (uptime, free heap, client count, brightness, request count)

## Hardware

- Seeed Studio XIAO ESP32S3
- USB-C data cable
- Phone, tablet, or computer with Wi-Fi

No additional LED or wiring is required.

> **Note:** This project controls the board's user LED on GPIO21, not the battery-charge indicator. GPIO21 can conflict with the microSD chip-select function on certain XIAO ESP32S3 variants, so avoid concurrent use.

## Software Requirements

- Arduino IDE 2.x
- Espressif **esp32** boards platform, **version 3.x** (the sketch uses the `ledcAttach(pin, frequency, resolution)` and `ledcWrite(pin, duty)` APIs)
- `WiFi.h` and `WebServer.h` (provided by the ESP32 Arduino core)

## Wi-Fi Configuration

| Setting | Default |
|---|---|
| Mode | Access Point (`WIFI_AP`) |
| SSID | `XIAO-LED-Control` |
| Password | `12345678` |
| AP IP address | `192.168.4.1` |
| Web page | `http://192.168.4.1` |
| HTTP port | `80` |
| LED pin | `GPIO21` |
| PWM frequency | `5000 Hz` |
| PWM resolution | `8 bits` |
| Serial baud rate | `115200` |

**Security:** Change `apPassword` to a stronger password before using the device in shared spaces. The HTTP interface has no separate authentication; any client connected to the AP can control the LED.

## Setup

1. Install Arduino IDE and the **esp32** board package (version 3.x) in **Boards Manager**.
2. Connect the XIAO ESP32S3 to your computer using a **data-capable** USB-C cable.
3. Select **Tools → Board → Seeed XIAO ESP32S3** (board entry names can vary by package version).
4. Select the appropriate serial port under **Tools → Port**. If necessary, enable **USB CDC On Boot** to view USB serial logs.
5. Open your project `.ino` file containing the AP-mode web server and Serial Monitor code.
6. Adjust `apSSID` and `apPassword` if desired.
7. Click **Upload**.
8. Open **Tools → Serial Monitor** and set the baud rate to **115200**.
9. On a phone or laptop, connect to **XIAO-LED-Control** with the configured password.
10. Navigate to **http://192.168.4.1** and move the slider or use the ON/OFF buttons.

A phone may display **“Connected without internet”**. This is expected: the ESP32S3 is providing a local network rather than an internet connection. If your phone switches back to mobile data, stay connected to the ESP32S3 Wi-Fi network and open the IP address directly.

## How It Works

```text
Phone / Laptop
    |
    | Wi-Fi (XIAO-LED-Control)
    v
XIAO ESP32S3 Access Point (192.168.4.1)
    |
    | HTTP GET /brightness?value=75
    v
WebServer → PWM (GPIO21) → Built-in user LED
    |
    v
Serial Monitor diagnostics (115200 baud)
```

The slider sends requests such as:

```http
GET /brightness?value=75
```

The sketch maps the requested brightness (0–100%) to an **inverted 8-bit PWM duty cycle**, because the onboard user LED is active-low:

| Brightness | Approx. PWM duty (0–255) | Effect |
|---|---:|---|
| 0% | 255 | Off |
| 25% | 191 | Dim |
| 50% | 128 | Medium |
| 75% | 64 | Bright |
| 100% | 0 | Maximum |

## Serial Monitoring

Open the Serial Monitor at **115200 baud**. The monitored sketch reports:

- LED brightness transitions and the corresponding PWM duty value
- AP SSID, IP address, and MAC address on startup
- HTTP request count, requesting client IP, and response status for brightness requests
- Changes in the number of connected Wi-Fi devices
- A status report every five seconds with uptime, free heap memory, LED brightness, HTTP requests, and client count

Example (illustrative; actual addresses and memory values vary):

```text
[WIFI] Starting Access Point...
[WIFI] Access Point started!
[WIFI] SSID: XIAO-LED-Control
[WIFI] IP Address: 192.168.4.1
[SERVER] HTTP server started
[READY] http://192.168.4.1
[WIFI] Client connected
[WIFI] Connected clients: 1
[HTTP] Request #1
[HTTP] Client IP: 192.168.4.2
[LED] Brightness: 50% -> 75%
[PWM] Duty: 64 / 255
[HTTP] Response: 200 OK
======== SYSTEM STATUS ========
Uptime: 15 seconds
Connected Clients: 1
LED Brightness: 75%
Free Heap: 245000 bytes
HTTP Requests: 1
===============================
```

The connection log detects **changes in total connected client count**; it does not identify each client by name.

## Troubleshooting

| Problem | What to check |
|---|---|
| AP network not visible | Verify upload completed and the AP-start messages appear in Serial Monitor. Reset the board if needed. |
| Webpage does not load | Connect to the ESP32S3's Wi-Fi and use `http://192.168.4.1` (not HTTPS). |
| Phone says no internet | Expected in AP mode; keep the phone connected to the ESP32S3 AP. |
| LED does not respond | Confirm **GPIO21**, active-low inversion, and **esp32 core 3.x**. Check Serial Monitor for brightness requests. |
| `ledcAttach` not found | Upgrade the ESP32 board package to version 3.x. |
| No Serial Monitor output | Choose **115200 baud**, correct port, and enable **USB CDC On Boot** if needed; reset the board. |
| Board not detected | Try a USB-C cable that supports data, not just charging. |

## Limitations

- The HTTP server is accessible only to devices connected to the ESP32S3 AP.
- The interface is plain HTTP, without user login or TLS.
- The provided monitoring code records brightness API requests, not every possible HTTP request.
- Logs and PWM operation depend on the full `.ino` sketch being present; this README is documentation, not a substitute for the firmware.

## References

- [Seeed Studio — Getting Started with XIAO ESP32S3](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- [Espressif — Arduino-ESP32 LEDC API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html)
- [Espressif — Arduino-ESP32 Wi-Fi AP API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)

## License

No license has been selected for this project. Add a `LICENSE` file if you intend to define reuse permissions.
