#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "led_engine.h"
#include "astronomy.h"

class LampWebServer {
public:
    LampWebServer(LedEngine& ledEngine, AstronomyEngine& astroEngine);

    void begin();
    void update(const MoonTelemetry& moon, const SunTelemetry& sun);

    void broadcastState();
    void broadcastTelemetry(const MoonTelemetry& moon, const SunTelemetry& sun);

private:
    WebServer _httpServer;
    WebSocketsServer _wsServer;
    LedEngine& _leds;
    AstronomyEngine& _astro;

    unsigned long _lastTeleBroadcast;

    void handleRoot();
    void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    void handleJsonCommand(uint8_t clientNum, const String& jsonStr);
};
