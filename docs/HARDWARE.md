# Hardware Architecture & Wiring Guide

## Components

| Component | Description | Specification |
| :--- | :--- | :--- |
| **Microcontroller** | ESP-WROOM-32 (NodeMCU / DevKit V1) | Dual-core 240 MHz, 4MB Flash, Wi-Fi 2.4 GHz |
| **LED Strip** | FCOB SPI RGB IC Addressable LED Strip | 5V DC, continuous phosphor glow (no diode hotspots), WS2811/WS2812B compatible protocol |
| **Power Supply** | Laptop USB-C / USB-A Power Adapter | 5V DC, 3.0A (15W) |
| **Power Breakout** | USB-C connector breakout / TP4056 board | Direct tap on VBUS (+5V) and GND |
| **Optional IR Kit** | HX1838 IR Receiver & NEC Remote | 38 kHz receiver, CR2032/CR2025 battery |

## Pinout Map

```
  ESP32 DevKit                  External Connection
  ======================================================
  VIN (5V)       <---------->   +5V from Power Supply & LED +5V
  GND            <---------->   GND from Power Supply & LED GND
  GPIO 18 (D18)  ---------->   [330Ω Resistor] -> LED DATA (DIN)
  GPIO 19 (D19)  <----------   HX1838 IR Receiver OUT (Optional)
  3V3            ---------->   HX1838 VCC (Optional)
```

## Power Budget & Electrical Guidelines

* **Circumference for 50 cm Lamp:** $\approx 157\text{ cm}$.
* **FCOB Strip Draw:** $\approx 10\text{W/m}$ to $14\text{W/m}$ at 100% full white.
* **Protection Limit:** Software power limiter set to **2500 mA (12.5W)** via FastLED:
  ```cpp
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2500);
  ```
* **Common Ground:** ESP32 ground, LED power ground, and power supply negative rail MUST be tied to a single common GND.
