/**
 * @file Moon.ino
 * @brief Smart Illuminated Lunar Wall Lamp - Core Firmware
 * @author sergeapollonov
 * @license MIT
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "src/astronomy.h"
#include "src/time_sync.h"
#include "src/led_engine.h"
#include "src/settings_manager.h"
#include "src/web_server.h"

// Локальный файл с настройками сети (добавлен в .gitignore)
#if __has_include("credentials.h")
#include "credentials.h"
#else
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
#endif

// Координаты по умолчанию (Варшава: 52.2297° N, 21.0122° E)
const double DEFAULT_LAT = 52.2297;
const double DEFAULT_LON = 21.0122;

const int LED_STATUS_PIN = 2; // Синий диод ESP32

AstronomyEngine astro(DEFAULT_LAT, DEFAULT_LON);
TimeSync timeSync;
LedEngine ledEngine;
SettingsManager settingsManager;
LampWebServer webServer(ledEngine, astro, settingsManager);

unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL_MS = 5 * 60 * 1000UL; // Перерасчет астрономии каждые 5 минут

int lastSunsetTriggerDay = -1;
int lastAutoOffTriggerDay = -1;
int lastSunriseTriggerDay = -1;
unsigned long lastScheduleCheckTime = 0;
const unsigned long SCHEDULE_CHECK_INTERVAL_MS = 10000; // Проверка расписания каждые 10 секунд

MoonTelemetry currentMoon;
SunTelemetry currentSun;

void checkScheduleAutomation(const SunTelemetry& sun) {
    if (!timeSync.isSynchronized()) return;

    struct tm localTm = timeSync.getLocalTime();
    int currentDay = localTm.tm_yday;
    int currentMinutes = localTm.tm_hour * 60 + localTm.tm_min;

    LampState s = ledEngine.getState();

    // 1. Автоматическое включение на закате
    if (s.autoSunset) {
        if (currentMinutes >= sun.sunsetMinutes && currentMinutes < (sun.sunsetMinutes + 45)) {
            if (lastSunsetTriggerDay != currentDay) {
                lastSunsetTriggerDay = currentDay;
                if (!s.power) {
                    Serial.printf("[AUTO] Sunset reached (%02d:%02d) -> Turning Lamp ON\n",
                                  sun.sunsetHour, sun.sunsetMinute);
                    ledEngine.setPower(true);
                    webServer.broadcastState();
                }
            }
        }
    }

    // 2. Автоматическое отключение по ночному таймеру (например, 01:00)
    if (s.autoOff) {
        int targetOffMinutes = s.autoOffHour * 60 + s.autoOffMinute;
        if (currentMinutes >= targetOffMinutes && currentMinutes < (targetOffMinutes + 45)) {
            if (lastAutoOffTriggerDay != currentDay) {
                lastAutoOffTriggerDay = currentDay;
                if (s.power) {
                    Serial.printf("[AUTO] Sleep timer reached (%02d:%02d) -> Turning Lamp OFF\n",
                                  s.autoOffHour, s.autoOffMinute);
                    ledEngine.setPower(false);
                    webServer.broadcastState();
                }
            }
        }
    }

    // 3. Автоматическое отключение на рассвете
    if (s.autoSunset) {
        if (currentMinutes >= sun.sunriseMinutes && currentMinutes < (sun.sunriseMinutes + 45)) {
            if (lastSunriseTriggerDay != currentDay) {
                lastSunriseTriggerDay = currentDay;
                if (s.power) {
                    Serial.printf("[AUTO] Sunrise reached (%02d:%02d) -> Turning Lamp OFF\n",
                                  sun.sunriseHour, sun.sunriseMinute);
                    ledEngine.setPower(false);
                    webServer.broadcastState();
                }
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(LED_STATUS_PIN, OUTPUT);

    Serial.println("\n");
    Serial.println("==========================================================");
    Serial.println("       🌕 LUNAR LAMP — SMART WALL ILLUMINATION 🌕         ");
    Serial.println("           Stage 2: Artisan Web UI & LED Engine           ");
    Serial.println("==========================================================");

    // Загрузка настроек из энергонезависимой памяти (NVS)
    settingsManager.begin(ledEngine, astro);

    // Инициализация светодиодного движка (по умолчанию 252 диода FCOB, старт на 12 часов)
    ledEngine.begin(ledEngine.getState().ledCount);
    Serial.printf("[OK] FastLED Engine initialized (%u LEDs, Power Cap: 5V 2500mA).\n",
                  ledEngine.getState().ledCount);

    // Подключение к Wi-Fi
    if (String(WIFI_SSID) != "YOUR_WIFI_SSID") {
        Serial.print("Connecting to Wi-Fi: ");
        Serial.println(WIFI_SSID);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 25) {
            delay(400);
            digitalWrite(LED_STATUS_PIN, !digitalRead(LED_STATUS_PIN));
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n[OK] Wi-Fi connected! IP address: " + WiFi.localIP().toString());
            digitalWrite(LED_STATUS_PIN, HIGH);
            
            // Синхронизация времени NTP
            timeSync.begin();
            Serial.println("[OK] NTP Time Synchronization started.");

            // Запуск mDNS (доступно по адресу http://moon.local)
            if (MDNS.begin("moon")) {
                Serial.println("[OK] mDNS responder started: http://moon.local");
            }

            // Запуск веб-сервера и WebSockets
            webServer.begin();
        } else {
            Serial.println("\n[WARN] Wi-Fi connection timed out. Starting fallback AP mode...");
            WiFi.mode(WIFI_AP);
            WiFi.softAP("Moon-Lamp-Setup", "12345678");
            Serial.println("[OK] Fallback AP created: 'Moon-Lamp-Setup' (IP: 192.168.4.1)");
            webServer.begin();
            digitalWrite(LED_STATUS_PIN, LOW);
        }
    } else {
        Serial.println("[INFO] Wi-Fi credentials not configured in credentials.h.");
        Serial.println("[INFO] Starting Access Point: 'Moon-Lamp-Setup' (192.168.4.1)");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("Moon-Lamp-Setup", "12345678");
        webServer.begin();
    }

    // Первичный расчет астрономии
    time_t utcNow = timeSync.getUtcEpoch();
    struct tm localTm = timeSync.getLocalTime();
    int tzOffset = timeSync.getTimezoneOffsetMinutes();

    currentMoon = astro.calculateMoon(utcNow);
    currentSun = astro.calculateSun(localTm, tzOffset);

    Serial.printf("[LUNAR] Initial Phase: %s | Illum: %.1f%% | Angle: %.1f° | Sunset: %02d:%02d\n",
                  astro.getAsciiMoon(currentMoon.phaseType),
                  currentMoon.illumination * 100.0,
                  currentMoon.phaseAngleDeg,
                  currentSun.sunsetHour, currentSun.sunsetMinute);

    // Если плата запустилась ночью после времени отбоя, но до рассвета
    if (timeSync.isSynchronized()) {
        LampState s = ledEngine.getState();
        int currentMins = localTm.tm_hour * 60 + localTm.tm_min;
        int targetOffMins = s.autoOffHour * 60 + s.autoOffMinute;
        if (s.autoOff && currentMins >= targetOffMins && currentMins < currentSun.sunriseMinutes) {
            Serial.printf("[AUTO] Booted during night sleep period (%02d:%02d) -> Setting Lamp OFF\n",
                          s.autoOffHour, s.autoOffMinute);
            ledEngine.setPower(false);
            lastAutoOffTriggerDay = localTm.tm_yday;
        }
    }

    lastTelemetryTime = millis();
}

void loop() {
    unsigned long now = millis();

    // Обработка отложенного сохранения настроек в Flash (debouncing 2 сек)
    settingsManager.update(ledEngine, astro);

    // Обновление веб-сервера (HTTP клиенты и WebSockets)
    webServer.update(currentMoon, currentSun);

    // Обновление светодиодного движка (рендеринг фазы или эффекта)
    ledEngine.update(currentMoon, currentSun);

    // Проверка автоматических событий расписания (закат / ночной таймер / рассвет)
    if (now - lastScheduleCheckTime >= SCHEDULE_CHECK_INTERVAL_MS) {
        lastScheduleCheckTime = now;
        checkScheduleAutomation(currentSun);
    }

    // Периодический перерасчет астрономии (каждые 5 минут)
    if (now - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
        lastTelemetryTime = now;

        time_t utcNow = timeSync.getUtcEpoch();
        struct tm localTm = timeSync.getLocalTime();
        int tzOffset = timeSync.getTimezoneOffsetMinutes();

        currentMoon = astro.calculateMoon(utcNow);
        currentSun = astro.calculateSun(localTm, tzOffset);

        Serial.printf("[LUNAR] Phase: %s | Illum: %.1f%% | Angle: %.1f° | Sunset: %02d:%02d\n",
                      astro.getAsciiMoon(currentMoon.phaseType),
                      currentMoon.illumination * 100.0,
                      currentMoon.phaseAngleDeg,
                      currentSun.sunsetHour, currentSun.sunsetMinute);

        webServer.broadcastTelemetry(currentMoon, currentSun);
    }
}
