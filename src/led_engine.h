#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include "astronomy.h"

// Максимальное поддерживаемое число светодиодов для буфера памяти
#define MAX_LEDS 300
#define LED_DATA_PIN 18

enum LampMode {
    LAMP_MODE_LIVE_ASTRONOMY = 0, // Автоматическая фаза Луны и суточный цикл
    LAMP_MODE_MANUAL_PHASE   = 1, // Ручной выбор фазы и угла
    LAMP_MODE_BREATHE        = 2, // Лунное дыхание (плавная пульсация)
    LAMP_MODE_ECLIPSE        = 3, // Солнечное затмение (огненная корона)
    LAMP_MODE_AURORA         = 4, // Северное сияние (интерференционные волны)
    LAMP_MODE_SOLID          = 5  // Сплошной ночник
};

struct LampState {
    bool power;                 // Вкл/Выкл
    uint8_t brightness;         // Яркость: 0 .. 255
    uint16_t kelvin;            // Цветовая температура: 2000K .. 6500K
    LampMode mode;              // Текущий режим
    double manualPhaseAngle;    // Ручной угол фазы (0..360°)
    uint16_t ledCount;          // Фактическое количество светодиодов в лампе
    int16_t zeroAngleOffset;    // Смещение нулевого диода (0..360°)
};

class LedEngine {
public:
    LedEngine();

    void begin(uint16_t ledCount = 60);
    void update(const MoonTelemetry& moon, const SunTelemetry& sun);

    // Управление состоянием
    void setPower(bool on);
    void setBrightness(uint8_t brightness);
    void setKelvin(uint16_t kelvin);
    void setMode(LampMode mode);
    void setManualPhase(double phaseAngleDeg);
    void setLedCount(uint16_t count);
    void setZeroAngleOffset(int16_t offsetDeg);

    LampState getState() const { return _state; }

private:
    CRGB _leds[MAX_LEDS];
    LampState _state;

    void renderAstronomyPhase(double phaseAngleDeg, double illumination, bool isWaxing, DayPhaseType dayPhase);
    void renderBreathe(CRGB baseColor);
    void renderEclipse();
    void renderAurora();
    void renderSolid(CRGB color);

    CRGB kelvinToRGB(uint16_t kelvin);
    double getLedAngleDeg(int index);
    double getAngularDistance(double a1, double a2);
};
