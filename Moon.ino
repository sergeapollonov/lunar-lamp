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
LampWebServer webServer(ledEngine, astro);

unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL_MS = 5 * 60 * 1000UL; // Перерасчет астрономии каждые 5 минут

MoonTelemetry currentMoon;
SunTelemetry currentSun;

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(LED_STATUS_PIN, OUTPUT);

    Serial.println("\n");
    Serial.println("==========================================================");
    Serial.println("       🌕 LUNAR LAMP — SMART WALL ILLUMINATION 🌕         ");
    Serial.println("           Stage 2: Artisan Web UI & LED Engine           ");
    Serial.println("==========================================================");

    // Инициализация светодиодного движка (по умолчанию 60 диодов, меняется в веб-интерфейсе)
    ledEngine.begin(60);
    Serial.println("[OK] FastLED Engine initialized (Power Cap: 5V 2500mA).");

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

    lastTelemetryTime = millis();
}

void loop() {
    unsigned long now = millis();

    // Обновление веб-сервера (HTTP клиенты и WebSockets)
    webServer.update(currentMoon, currentSun);

    // Обновление светодиодного движка (рендеринг фазы или эффекта)
    ledEngine.update(currentMoon, currentSun);

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
