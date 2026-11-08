/**
 * @file Moon.ino
 * @brief Smart Illuminated Lunar Wall Lamp - Core Firmware
 * @author sergeapollonov
 * @license MIT
 */

#include <Arduino.h>
#include <WiFi.h>
#include "src/astronomy.h"
#include "src/time_sync.h"

// Если есть локальный файл credentials.h с настройками Wi-Fi - подключаем его
#if __has_include("credentials.h")
#include "credentials.h"
#else
// Замените на ваши учетные данные домашнего Wi-Fi
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
#endif

// Координаты по умолчанию (Варшава: 52.2297° N, 21.0122° E)
const double DEFAULT_LAT = 52.2297;
const double DEFAULT_LON = 21.0122;

const int LED_STATUS_PIN = 2; // Синий диод ESP32

AstronomyEngine astro(DEFAULT_LAT, DEFAULT_LON);
TimeSync timeSync;

unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL_MS = 5000;

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(LED_STATUS_PIN, OUTPUT);

    Serial.println("\n");
    Serial.println("==========================================================");
    Serial.println("       🌕 LUNAR LAMP — SMART WALL ILLUMINATION 🌕         ");
    Serial.println("              Stage 1: Core Astronomy Engine             ");
    Serial.println("==========================================================");

    // Подключение к Wi-Fi
    if (String(WIFI_SSID) != "YOUR_WIFI_SSID") {
        Serial.print("Connecting to Wi-Fi: ");
        Serial.println(WIFI_SSID);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            digitalWrite(LED_STATUS_PIN, !digitalRead(LED_STATUS_PIN));
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n[OK] Wi-Fi connected! IP: " + WiFi.localIP().toString());
            digitalWrite(LED_STATUS_PIN, HIGH);
            
            // Запуск синхронизации точного времени по NTP
            timeSync.begin();
            Serial.println("[OK] NTP Time Synchronization started.");
        } else {
            Serial.println("\n[WARN] Wi-Fi connection timed out. Running in offline demo mode.");
            digitalWrite(LED_STATUS_PIN, LOW);
        }
    } else {
        Serial.println("[INFO] Wi-Fi credentials not configured yet.");
        Serial.println("[INFO] You can set WIFI_SSID and WIFI_PASSWORD in Moon.ino or credentials.h");
        Serial.println("[INFO] Running astronomy calculation on default system epoch.\n");
    }
}

void loop() {
    unsigned long now = millis();
    if (now - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
        lastTelemetryTime = now;

        time_t utcNow = timeSync.getUtcEpoch();
        struct tm localTm = timeSync.getLocalTime();
        int tzOffset = timeSync.getTimezoneOffsetMinutes();

        MoonTelemetry moon = astro.calculateMoon(utcNow);
        SunTelemetry sun = astro.calculateSun(localTm, tzOffset);

        Serial.println("----------------------------------------------------------");
        Serial.printf(" 🕒 Локальное время:  %04d-%02d-%02d %02d:%02d:%02d\n",
                      localTm.tm_year + 1900, localTm.tm_mon + 1, localTm.tm_mday,
                      localTm.tm_hour, localTm.tm_min, localTm.tm_sec);
        Serial.printf(" 🌘 Фаза Луны:        %s %s\n", astro.getAsciiMoon(moon.phaseType), moon.phaseNameRu);
        Serial.printf(" 💡 Освещенность:     %.1f%%\n", moon.illumination * 100.0);
        Serial.printf(" 📐 Фазовый угол:     %.1f° (Свет %s)\n", 
                      moon.phaseAngleDeg, moon.isWaxing ? "СПРАВА (Растущая)" : "СЛЕВА (Убывающая)");
        Serial.printf(" ⏳ Возраст Луны:     %.1f из 29.5 дней\n", moon.ageDays);
        Serial.printf(" 🌅 Восход Солнца:    %02d:%02d\n", sun.sunriseHour, sun.sunriseMinute);
        Serial.printf(" 🌇 Закат Солнца:     %02d:%02d\n", sun.sunsetHour, sun.sunsetMinute);
        Serial.printf(" ☀️ Фаза суток:       %s\n", sun.dayPhaseNameRu);
        Serial.println("----------------------------------------------------------");
    }

    // Ритмичное мерцание светодиода статуса (heartbeat)
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 1000) {
        lastBlink = millis();
        digitalWrite(LED_STATUS_PIN, !digitalRead(LED_STATUS_PIN));
    }
}
