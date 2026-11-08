#include "web_server.h"
#include "web_assets.h"

LampWebServer::LampWebServer(LedEngine& ledEngine, AstronomyEngine& astroEngine)
    : _httpServer(80), _wsServer(81), _leds(ledEngine), _astro(astroEngine), _lastTeleBroadcast(0) {}

void LampWebServer::begin() {
    // HTTP маршрут отдачи сжатого веб-интерфейса
    _httpServer.on("/", HTTP_GET, [this]() {
        this->handleRoot();
    });

    _httpServer.onNotFound([this]() {
        _httpServer.sendHeader("Location", "/");
        _httpServer.send(302, "text/plain", "");
    });

    _httpServer.begin();

    // Запуск WebSockets сервера на порту 81
    _wsServer.begin();
    _wsServer.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        this->onWebSocketEvent(num, type, payload, length);
    });

    Serial.println("[OK] HTTP Web Server started on port 80");
    Serial.println("[OK] WebSockets Server started on port 81");
}

void LampWebServer::handleRoot() {
    _httpServer.sendHeader("Content-Encoding", "gzip");
    _httpServer.send_P(200, "text/html", (const char*)INDEX_HTML_GZ, INDEX_HTML_GZ_LEN);
}

void LampWebServer::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
            IPAddress ip = _wsServer.remoteIP(num);
            Serial.printf("[WS] Client #%u connected from %s\n", num, ip.toString().c_str());
            broadcastState();
            break;
        }
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u disconnected\n", num);
            break;

        case WStype_TEXT: {
            String msg = String((char*)payload).substring(0, length);
            handleJsonCommand(num, msg);
            break;
        }
        default:
            break;
    }
}

void LampWebServer::handleJsonCommand(uint8_t clientNum, const String& jsonStr) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, jsonStr);
    if (err) {
        Serial.printf("[WS ERR] JSON parse failed: %s\n", err.c_str());
        return;
    }

    const char* cmd = doc["cmd"] | "";

    if (strcmp(cmd, "getState") == 0) {
        broadcastState();
    } else if (strcmp(cmd, "setPower") == 0) {
        _leds.setPower(doc["power"] | true);
        broadcastState();
    } else if (strcmp(cmd, "setBrightness") == 0) {
        _leds.setBrightness(doc["brightness"] | 180);
        broadcastState();
    } else if (strcmp(cmd, "setKelvin") == 0) {
        _leds.setKelvin(doc["kelvin"] | 4000);
        broadcastState();
    } else if (strcmp(cmd, "setMode") == 0) {
        _leds.setMode((LampMode)(doc["mode"] | 0));
        broadcastState();
    } else if (strcmp(cmd, "setManualPhase") == 0) {
        _leds.setManualPhase(doc["angle"] | 180.0);
        broadcastState();
    } else if (strcmp(cmd, "setLedCount") == 0) {
        _leds.setLedCount(doc["count"] | 60);
        broadcastState();
    } else if (strcmp(cmd, "setOffsetAngle") == 0) {
        _leds.setZeroAngleOffset(doc["offset"] | 0);
        broadcastState();
    }
}

void LampWebServer::broadcastState() {
    LampState s = _leds.getState();

    JsonDocument doc;
    JsonObject stateObj = doc["state"].to<JsonObject>();
    stateObj["power"] = s.power;
    stateObj["brightness"] = s.brightness;
    stateObj["kelvin"] = s.kelvin;
    stateObj["mode"] = (int)s.mode;
    stateObj["manualAngle"] = s.manualPhaseAngle;
    stateObj["ledCount"] = s.ledCount;
    stateObj["offsetAngle"] = s.zeroAngleOffset;

    String out;
    serializeJson(doc, out);
    _wsServer.broadcastTXT(out);
}

void LampWebServer::broadcastTelemetry(const MoonTelemetry& moon, const SunTelemetry& sun) {
    JsonDocument doc;
    JsonObject tele = doc["telemetry"].to<JsonObject>();

    tele["illumination"] = moon.illumination;
    tele["phaseAngle"] = moon.phaseAngleDeg;
    tele["isWaxing"] = moon.isWaxing;
    tele["phaseName"] = moon.phaseNameRu;
    tele["ageDays"] = moon.ageDays;

    char sunBuf[16];
    snprintf(sunBuf, sizeof(sunBuf), "%02d:%02d", sun.sunriseHour, sun.sunriseMinute);
    tele["sunrise"] = String(sunBuf);

    snprintf(sunBuf, sizeof(sunBuf), "%02d:%02d", sun.sunsetHour, sun.sunsetMinute);
    tele["sunset"] = String(sunBuf);

    tele["dayPhase"] = sun.dayPhaseNameRu;

    String out;
    serializeJson(doc, out);
    _wsServer.broadcastTXT(out);
}

void LampWebServer::update(const MoonTelemetry& moon, const SunTelemetry& sun) {
    _httpServer.handleClient();
    _wsServer.loop();

    unsigned long now = millis();
    // Рассылка астрономической телеметрии каждые 3 секунды
    if (now - _lastTeleBroadcast >= 3000) {
        _lastTeleBroadcast = now;
        broadcastTelemetry(moon, sun);
    }
}
