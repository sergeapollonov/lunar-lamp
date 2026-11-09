# 🌕 Lunar Lamp — Smart Illuminated Lunar Wall Lamp

An open-source smart interior wall lamp that physically mirrors the real-time moon phase and astronomical twilight cycles using an **ESP32** and a high-density **FCOB Addressable LED strip**.

Based on the 3D-printable model: [Large Illuminated Lunar Wall Lamp V2 on MakerWorld](https://makerworld.com/pl/models/2320985-large-illuminated-lunar-wall-lamp-version-2?from=search#profileId-2534929).

---

## ✨ Features

* 🌘 **Real-Time Astronomical Engine:** Uses Meeus algorithms to calculate lunar phase, illumination percentage ($0.0 \dots 1.0$), and phase angle locally on the ESP32.
* 🌅 **Solar Twilight & Auto-Dimming:** Automatically computes sunrise and sunset for your location, smoothly illuminating as dusk sets in and turning off at dawn.
* 💡 **Continuous FCOB Neon Illumination:** Eliminates LED hotspots with seamless Chip-on-Board addressable RGB lighting.
* 📱 **Crafted Web Interface (No AI Slop):** Self-contained, responsive, obsidian dark-themed web control panel running directly on the ESP32 via WebSockets with zero external dependencies.
* 💾 **Persistent Flash Storage (NVS):** Automatically remembers your brightness, color temperature, 252-LED calibration, and night sleep schedule across power cuts with safe write debouncing.
* 🔄 **Over-The-Air (OTA) Wireless Updates:** Upload compiled firmware binaries directly from any browser via `/update` without physical cables.
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
- [x] **Stage 1:** Core Astronomical Engine (Julian date, Moon phase angle, Illumination %, Sunrise/Sunset).
- [x] **Stage 2:** LED Engine & Geometric Ring Mapping (Terminator anti-aliasing, color temperature 2200K–6500K, 252 FCOB, 12 o'clock zenith alignment).
- [x] **Stage 3:** Premium Web UI (Photorealistic NASA LROC surface relief, interactive Moon dial, 8 complication dials, sunset auto-on & night sleep timer).
- [x] **Stage 4:** WebSockets & Persistent Flash Storage (NVS / Preferences with 2s debounce, factory reset, bi-directional state sync).
- [x] **Stage 5:** Hardware assembly & wiring manual, Web OTA wireless update engine.

---

## 📜 Contributing & Pull Requests

Development strictly follows the Pull Request workflow outlined in [docs/pr-workflow.md](docs/pr-workflow.md). All new features and fixes are submitted via PRs with verification evidence.

## 📄 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
