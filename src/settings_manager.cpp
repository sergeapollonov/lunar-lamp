#include "settings_manager.h"

static const char* NVS_NAMESPACE = "lunar_lamp";

SettingsManager::SettingsManager()
    : _pendingSave(false), _lastChangeTime(0) {}

void SettingsManager::begin(LedEngine& leds, AstronomyEngine& astro) {
    if (!_prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[NVS ERR] Failed to open Preferences namespace");
        return;
    }

    bool isInit = _prefs.getBool("init", false);
    if (!isInit) {
        Serial.println("[NVS] First boot detected. Initializing Flash with default settings...");
        _prefs.putBool("init", true);
        writeToFlash(leds.getState(), astro.getLatitude(), astro.getLongitude());
        return;
    }

    Serial.println("[NVS] Loading persisted settings from Flash...");

    bool power = _prefs.getBool("power", true);
    uint8_t brightness = _prefs.getUChar("bright", 180);
    uint16_t kelvin = _prefs.getUShort("kelvin", 4000);
    uint8_t mode = _prefs.getUChar("mode", 0);
    double manAngle = _prefs.getDouble("man_ang", 180.0);
    uint16_t ledCount = _prefs.getUShort("led_cnt", 252);
    int16_t offset = _prefs.getShort("offset", 0);
    bool autoSunset = _prefs.getBool("auto_sun", true);
    bool autoOff = _prefs.getBool("auto_off", true);
    uint8_t offH = _prefs.getUChar("off_h", 1);
    uint8_t offM = _prefs.getUChar("off_m", 0);
    double lat = _prefs.getDouble("lat", 52.2297);
    double lon = _prefs.getDouble("lon", 21.0122);

    _prefs.end();

    leds.setPower(power);
    leds.setBrightness(brightness);
    leds.setKelvin(kelvin);
    leds.setMode((LampMode)mode);
    leds.setManualPhase(manAngle);
    leds.setLedCount(ledCount);
    leds.setZeroAngleOffset(offset);
    leds.setAutoSunset(autoSunset);
    leds.setAutoOff(autoOff);
    leds.setAutoOffTime(offH, offM);

    astro.setCoordinates(lat, lon);

    Serial.printf("[NVS OK] Restored: %u LEDs, %uK, Bright: %u, Mode: %u, Sunset: %s, Sleep: %02d:%02d\n",
                  ledCount, kelvin, brightness, mode, autoSunset ? "ON" : "OFF", offH, offM);
}

void SettingsManager::scheduleSave() {
    _pendingSave = true;
    _lastChangeTime = millis();
}

void SettingsManager::commitNow(const LedEngine& leds, const AstronomyEngine& astro) {
    _pendingSave = false;
    writeToFlash(leds.getState(), astro.getLatitude(), astro.getLongitude());
}

void SettingsManager::update(const LedEngine& leds, const AstronomyEngine& astro) {
    if (_pendingSave && (millis() - _lastChangeTime >= SAVE_DEBOUNCE_MS)) {
        _pendingSave = false;
        writeToFlash(leds.getState(), astro.getLatitude(), astro.getLongitude());
    }
}

void SettingsManager::writeToFlash(const LampState& state, double lat, double lon) {
    if (!_prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[NVS ERR] Failed to open Preferences for writing");
        return;
    }

    _prefs.putBool("power", state.power);
    _prefs.putUChar("bright", state.brightness);
    _prefs.putUShort("kelvin", state.kelvin);
    _prefs.putUChar("mode", (uint8_t)state.mode);
    _prefs.putDouble("man_ang", state.manualPhaseAngle);
    _prefs.putUShort("led_cnt", state.ledCount);
    _prefs.putShort("offset", state.zeroAngleOffset);
    _prefs.putBool("auto_sun", state.autoSunset);
    _prefs.putBool("auto_off", state.autoOff);
    _prefs.putUChar("off_h", state.autoOffHour);
    _prefs.putUChar("off_m", state.autoOffMinute);
    _prefs.putDouble("lat", lat);
    _prefs.putDouble("lon", lon);

    _prefs.end();
    Serial.println("[NVS] Settings successfully committed to Flash.");
}

void SettingsManager::resetToDefaults(LedEngine& leds, AstronomyEngine& astro) {
    if (!_prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[NVS ERR] Failed to open Preferences for clearing");
        return;
    }
    _prefs.clear();
    _prefs.putBool("init", true);
    _prefs.end();

    leds.setPower(true);
    leds.setBrightness(180);
    leds.setKelvin(4000);
    leds.setMode(LAMP_MODE_LIVE_ASTRONOMY);
    leds.setManualPhase(180.0);
    leds.setLedCount(252);
    leds.setZeroAngleOffset(0);
    leds.setAutoSunset(true);
    leds.setAutoOff(true);
    leds.setAutoOffTime(1, 0);
    astro.setCoordinates(52.2297, 21.0122);

    commitNow(leds, astro);
    Serial.println("[NVS] Factory defaults restored.");
}
