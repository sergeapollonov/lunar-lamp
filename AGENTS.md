# Lunar Lamp — Agent Instructions & Guardrails

The **Lunar Lamp** is an open-source smart interior wall lamp project featuring an ESP32 microcontroller, high-density FCOB addressable LED strip, real-time astronomical calculation engine, and a premium self-contained web interface.

## Guardrails & Principles

1. **Safety First (Power Limits):**
   * The system is powered by a 5V 3A (15W) USB-C power supply.
   * Total current draw must NEVER exceed 2500 mA. FastLED hardware power limiter `FastLED.setMaxPowerInVoltsAndMilliamps(5, 2500)` must always be enforced.
   * Microcontroller GPIO outputs operate at 3.3V logic.

2. **No "AI Slop" in Web UI:**
   * Avoid generic, low-contrast, purple-gradient AI templates with default HTML inputs.
   * The web interface must be clean, atmospheric, mobile-first, and styled with a dark obsidian / space aesthetic.
   * Self-contained: Web assets must be stored in ESP32 Flash (Gzip / PROGMEM) with zero external CDN dependencies, so it works completely offline.

3. **PR-Driven Development:**
   * All changes must be developed in topic branches (`feature/...` or `fix/...`) and merged into `main` via GitHub Pull Requests.
   * Pull Request conventions, titles, and descriptions follow `docs/pr-workflow.md`.
   * Titles and PR descriptions are in **English**; conversation with the user is in **Russian**.

4. **Code Quality & Modular Architecture:**
   * Keep modules separated:
     - `astronomy.*`: Solar and lunar position calculation, phase fraction, sunrise/sunset.
     - `time_sync.*`: NTP and local RTC management.
     - `led_engine.*`: FastLED mapping, arc rendering, terminator anti-aliasing, effects.
     - `web_server.*`: Asynchronous HTTP / WebSockets server and embedded UI.
