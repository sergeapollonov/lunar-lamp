# 🌕 Lunar Lamp — Smart Illuminated Lunar Wall Lamp

An open-source smart interior wall lamp that physically mirrors the real-time moon phase and astronomical twilight cycles using an **ESP32** and a high-density **FCOB Addressable LED strip**.

Based on the 3D-printable model: [Large Illuminated Lunar Wall Lamp V2 on MakerWorld](https://makerworld.com/pl/models/2320985-large-illuminated-lunar-wall-lamp-version-2?from=search#profileId-2534929).

---

## ✨ Features

* 🌘 **Real-Time Astronomical Engine:** Uses Meeus algorithms to calculate lunar phase, illumination percentage ($0.0 \dots 1.0$), and phase angle locally on the ESP32.
* 🌅 **Solar Twilight & Auto-Dimming:** Automatically computes sunrise and sunset for your location, smoothly illuminating as dusk sets in and turning off at dawn.
* 💡 **Continuous FCOB Neon Illumination:** Eliminates LED hotspots with seamless Chip-on-Board addressable RGB lighting.
* 📱 **Crafted Web Interface (No AI Slop):** Self-contained, responsive, obsidian dark-themed web control panel running directly on the ESP32 via WebSockets with zero external dependencies.
* 🛡️ **Hardware Power Safety:** Built-in dynamic power budgeting capping power draw to 2500 mA on a 5V 3A power supply.

---

## 🛠️ Hardware Setup

Detailed wiring diagrams and electrical specifications can be found in [docs/HARDWARE.md](docs/HARDWARE.md).

* **Microcontroller:** ESP32 (ESP-WROOM-32)
* **LED Strip:** 5V FCOB SPI RGB IC (WS2811/WS2812B compatible protocol)
* **Power Supply:** 5V 3A (15W) USB-C power adapter
* **Control Pin:** GPIO 18 (Data out)

---

## 🚀 Roadmap & Development Stages

- [x] **Stage 0:** Repository setup, documentation, hardware specifications, and PR standards.
- [ ] **Stage 1:** Core Astronomical Engine (Julian date, Moon phase angle, Illumination %, Sunrise/Sunset).
- [ ] **Stage 2:** LED Engine & Geometric Ring Mapping (Terminator anti-aliasing, color temperature 2200K–6500K).
- [ ] **Stage 3:** Premium Web UI (Interactive SVG moon, touch sliders, effect modes).
- [ ] **Stage 4:** WebSockets & Persistent Flash Storage (NVS).
- [ ] **Stage 5:** Hardware assembly & physical calibration.

---

## 📜 Contributing & Pull Requests

Development strictly follows the Pull Request workflow outlined in [docs/pr-workflow.md](docs/pr-workflow.md). All new features and fixes are submitted via PRs with verification evidence.

## 📄 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
