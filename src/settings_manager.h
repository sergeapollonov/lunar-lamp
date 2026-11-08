#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "led_engine.h"
#include "astronomy.h"

class SettingsManager {
public:
    SettingsManager();

    // Загрузка сохраненных настроек из Flash при старте платы
    void begin(LedEngine& leds, AstronomyEngine& astro);

    // Запланировать отложенное сохранение (debouncing 2 секунды)
    void scheduleSave();

    // Немедленная запись в Flash
    void commitNow(const LedEngine& leds, const AstronomyEngine& astro);

    // Фоновый обработчик отложенного сохранения (вызывать в loop)
    void update(const LedEngine& leds, const AstronomyEngine& astro);

    // Сброс настроек к заводским дефолтам
    void resetToDefaults(LedEngine& leds, AstronomyEngine& astro);

private:
    Preferences _prefs;
    bool _pendingSave;
    unsigned long _lastChangeTime;
    static const unsigned long SAVE_DEBOUNCE_MS = 2000;

    void writeToFlash(const LampState& state, double lat, double lon);
};
