# 🌕 Lunar Lamp — Hardware Architecture & Assembly Manual (Stage 5)

Comprehensive electrical, mechanical, and assembly specifications for the **Lunar Lamp V2** (50 cm diameter 3D-printed wall lamp with 252 FCOB addressable LEDs).

---

## 📋 Bill of Materials (BOM)

### Required Components (Базовый комплект)

| Component | Quantity | Specification / Recommendation | Notes |
| :--- | :---: | :--- | :--- |
| **Microcontroller** | 1 | **ESP32 DevKit V1** (ESP-WROOM-32, 30 or 38-pin) | Dual-core 240 MHz, 4MB Flash, 2.4 GHz Wi-Fi |
| **Addressable LED Strip** | 1.57 m | **5V FCOB SPI RGB IC** (160 LEDs/meter) | WS2811 / WS2812B compatible protocol, continuous phosphor (zero hotspots) |
| **Power Supply** | 1 | **5V 3.0A (15W)** USB-C Power Adapter | High-quality branded PSU (e.g. Baseus, Ugreen, Anker) |
| **Power Breakout** | 1 | **USB-C Female Breakout Board** (or TP4056 USB-C pads) | Direct power tap on VBUS (+5V) and GND |
| **Power Wire** | ~2 m | **AWG 20 or AWG 18** stranded silicone wire | Flexible, low voltage drop for 2.5A current rails |
| **Signal Wire** | ~0.5 m | **AWG 24 or AWG 26** stranded wire | For GPIO 18 data and optional sensors |
| **3D Printed Housing** | 1 set | [MakerWorld Model #2320985 (V2)](https://makerworld.com/pl/models/2320985-large-illuminated-lunar-wall-lamp-version-2) | 50 cm outer ring, diffuser channel, electronics bay |

### Optional Components (Опционально — не обязательно для сборки)

| Component | Quantity | Specification | When to use / Purpose |
| :--- | :---: | :--- | :--- |
| **Electrolytic Capacitor** | 1 | **1000 µF, 6.3V or 16V** Low-ESR | *Опционально:* сглаживает броски тока при использовании нестабильных блоков питания. Не требуется при качественном адаптере 5V 3A и плавном программном включении. |
| **Damping Resistor** | 1 | **330 Ω to 470 Ω** (0.25W) | *Опционально:* гасит паразитные отражения сигнала в длинных проводах (> 30–50 см). В корпусе лампы провод короткий (5–10 см), поэтому GPIO 18 подключается **напрямую к DIN**. |
| **IR Receiver Kit** | 1 | **HX1838 + NEC Remote** | *Опционально:* для управления режимами с инфракрасного пульта (CR2032/CR2025). |

---

## ⚡ Electrical Schematic & Wiring Diagram

### Standard Direct Connection (Базовая схема — без лишних деталей)

В корпусе лампы контроллер ESP32 располагается в непосредственной близости от стыка ленты (длина сигнального провода всего 5–10 см). Все компоненты подключаются напрямую:

```
                 +5V USB-C Power Supply (5V 3.0A)
                         │               │
                        [+]             [-]
                         │               │
       ┌─────────────────┴─┐           ┌─┴─────────────────┐
       │   ESP32 DevKit    │           │                   │
       │                   │           │                   │
       │   VIN (5V in) ────┼───────────┤                   │
       │   GND ────────────┼───────────┤ Common Ground     │
       │                   │           │                   │
       │   GPIO 18 (Data)  │           │                   │
       └─────────┬─────────┘           │                   │
                 │ (Direct wire)       │                   │
                 ▼                     │                   │
       ┌───────────────────┐           │                   │
       │ FCOB LED Strip    │           │                   │
       │ Start (LED #0)    │           │                   │
       │   +5V ────────────┼───────────┘                   │
       │   DIN ────────────┘ (Direct to GPIO 18)           │
       │   GND ────────────────────────────────────────────┤
       │                   │                               │
       │   ... 252 LEDs    │                               │
       │   (1.57 meters)   │                               │
       │                   │                               │
       │ FCOB LED Strip    │                               │
       │ End (LED #251)    │                               │
       │   +5V (Injection)─┴───────────────────────────────┘ (Dual-ended power injection)
       │   GND (Injection)─────────────────────────────────┘
       └───────────────────┘
```

> [!NOTE] Опциональные элементы защиты (по желанию):
> Если у вас под рукой есть компоненты и вы хотите добавить дополнительную фильтрацию:
> * **Конденсатор 1000 µF:** припаивается параллельно питанию (плюс к +5V, минус к GND) около разъема USB-C.
> * **Резистор 330–470 Ω:** впаивается в разрыв провода между GPIO 18 и контактом DIN ленты (актуально только если провод длиннее 30 см).

---

## 📐 Strip Geometry & 12 O'Clock Zenith Alignment

The mathematical engine in `LedEngine` calculates terminator angles relative to a clock face:

* **Outer Circumference:** $L = 1.57\text{ m}$ (exact circumference for $D = 50\text{ cm}$: $50 \times \pi \approx 157.08\text{ cm}$).
* **Total LEDs:** $1.57\text{ m} \times 160\text{ LED/m} = 251.2\text{ LEDs} \implies \mathbf{252\text{ LEDs}}$.
* **Angular Resolution:** $360^\circ / 252 \approx \mathbf{1.43^\circ\text{ per diode}}$.

### Mounting Instructions:
1. **Zenith Point (12:00):** Glue the start of the strip (LED `#0`, with the `DIN` arrow pointing forward) at the **top dead center (12 o'clock)** of the lamp disc.
2. **Clockwise Direction:** Run the strip **clockwise** around the perimeter:
   * **LEDs 0 → 63:** Right quadrant (12:00 to 3:00 — Waxing Crescent / First Quarter illumination).
   * **LEDs 63 → 126:** Lower right quadrant (3:00 to 6:00 — Waxing Gibbous).
   * **LED #126:** Bottom dead center (**6 o'clock**).
   * **LEDs 126 → 189:** Lower left quadrant (6:00 to 9:00 — Waning Gibbous).
   * **LEDs 189 → 251:** Upper left quadrant (9:00 to 12:00 — Waning Crescent).
   * **LED #251:** Meets LED `#0` at **12 o'clock**.
3. **Dual-Ended Power Injection:** Solder both the start (`+5V`, `GND`) and end (`+5V`, `GND`) of the strip to the main power bus. This prevents resistance voltage drop across the 1.57m strip, ensuring uniform white brightness around the entire 360° circle.
4. **Software Fine-Tuning:** If the strip is glued slightly offset by a few degrees, use the **«Калибровка верхней точки (12 часов)»** slider in the Web UI to software-rotate the zero point without peeling the strip.

---

## 🛡️ Power Safety & Hardware Protection

1. **Hardware Power Limiter (FastLED):**
   * Configured in `src/led_engine.cpp`:
     ```cpp
     FastLED.setMaxPowerInVoltsAndMilliamps(5, 2500);
     ```
   * FastLED dynamically computes total current consumption in real time and scales down brightness if total power demands exceed **2500 mA (12.5 W)**.
2. **Power Budget Allocation:**
   * **ESP32 Active Wi-Fi & CPU:** $\sim 150 - 200\text{ mA}$.
   * **252 FCOB LEDs:** Up to $2300\text{ mA}$.
   * **Total Max Draw:** $\le 2500\text{ mA}$, leaving a safe $500\text{ mA}$ headroom on the 5V 3.0A USB-C power supply.
3. **Logic Levels:**
   * ESP32 GPIO operates at 3.3V logic. FCOB WS2811/WS2812 ICs running at 5V reliably detect 3.3V high logic levels directly without any level shifter or resistor when wire lengths are under 20 cm (GPIO 18 -> DIN directly).

---

## 🔄 Over-The-Air (OTA) Firmware Updates

Once the lamp is assembled and mounted on the wall, firmware updates can be installed wirelessly over Wi-Fi:

1. In Arduino IDE: **Sketch → Export Compiled Binary** (generates `Moon.ino.bin` in the `build/` directory).
2. Open your browser and navigate to:
   ```
   http://moon.local/update   (or http://<esp32-ip>/update)
   ```
3. Select `Moon.ino.bin` and click **«Установить обновление»**.
4. The web updater displays an animated progress bar ($0\% \to 100\%$) and automatically reboots the ESP32 in $\sim 15$ seconds.
