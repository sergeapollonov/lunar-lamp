#include "led_engine.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

LedEngine::LedEngine() {
    _state.power = true;
    _state.brightness = 180;
    _state.kelvin = 4000; // Нейтральный лунный свет
    _state.mode = LAMP_MODE_LIVE_ASTRONOMY;
    _state.manualPhaseAngle = 180.0;
    _state.ledCount = 252;
    _state.zeroAngleOffset = 0; // 0° = 12 часов (старт и конец ленты в верхней точке)
    _state.autoSunset = true;
    _state.autoOff = true;
    _state.autoOffHour = 1;     // По умолчанию отключение в 01:00
    _state.autoOffMinute = 0;
}

void LedEngine::begin(uint16_t ledCount) {
    if (ledCount > MAX_LEDS) ledCount = MAX_LEDS;
    _state.ledCount = ledCount;

    FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(_leds, MAX_LEDS);
    
    // ВАЖНО: Аппаратный лимитер питания для защиты 5V 3A блока
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 2500);
    FastLED.setBrightness(_state.brightness);
    FastLED.clear(true);
}

void LedEngine::setPower(bool on) {
    _state.power = on;
    if (!on) {
        FastLED.clear(true);
    }
}

void LedEngine::setBrightness(uint8_t brightness) {
    _state.brightness = brightness;
    FastLED.setBrightness(brightness);
}

void LedEngine::setKelvin(uint16_t kelvin) {
    if (kelvin < 2000) kelvin = 2000;
    if (kelvin > 6500) kelvin = 6500;
    _state.kelvin = kelvin;
}

void LedEngine::setMode(LampMode mode) {
    _state.mode = mode;
}

void LedEngine::setManualPhase(double phaseAngleDeg) {
    while (phaseAngleDeg < 0.0) phaseAngleDeg += 360.0;
    while (phaseAngleDeg >= 360.0) phaseAngleDeg -= 360.0;
    _state.manualPhaseAngle = phaseAngleDeg;
}

void LedEngine::setLedCount(uint16_t count) {
    if (count > MAX_LEDS) count = MAX_LEDS;
    if (count < 10) count = 10;
    _state.ledCount = count;
    FastLED.clear();
}

void LedEngine::setZeroAngleOffset(int16_t offsetDeg) {
    _state.zeroAngleOffset = offsetDeg;
}

void LedEngine::setAutoSunset(bool enabled) {
    _state.autoSunset = enabled;
}

void LedEngine::setAutoOff(bool enabled) {
    _state.autoOff = enabled;
}

void LedEngine::setAutoOffTime(uint8_t hour, uint8_t minute) {
    if (hour > 23) hour = 23;
    if (minute > 59) minute = 59;
    _state.autoOffHour = hour;
    _state.autoOffMinute = minute;
}

double LedEngine::getLedAngleDeg(int index) {
    double angle = (index * 360.0) / _state.ledCount + _state.zeroAngleOffset;
    angle = fmod(angle, 360.0);
    if (angle < 0.0) angle += 360.0;
    return angle;
}

double LedEngine::getAngularDistance(double a1, double a2) {
    double diff = fabs(a1 - a2);
    if (diff > 180.0) diff = 360.0 - diff;
    return diff;
}

CRGB LedEngine::kelvinToRGB(uint16_t kelvin) {
    double temp = kelvin / 100.0;
    double red, green, blue;

    // Red
    if (temp <= 66.0) {
        red = 255.0;
    } else {
        red = temp - 60.0;
        red = 329.698727446 * pow(red, -0.1332047592);
        if (red < 0.0) red = 0.0;
        if (red > 255.0) red = 255.0;
    }

    // Green
    if (temp <= 66.0) {
        green = temp;
        green = 99.4708025861 * log(green) - 161.1195681661;
        if (green < 0.0) green = 0.0;
        if (green > 255.0) green = 255.0;
    } else {
        green = temp - 60.0;
        green = 288.1221695283 * pow(green, -0.0755148492);
        if (green < 0.0) green = 0.0;
        if (green > 255.0) green = 255.0;
    }

    // Blue
    if (temp >= 66.0) {
        blue = 255.0;
    } else if (temp <= 19.0) {
        blue = 0.0;
    } else {
        blue = temp - 10.0;
        blue = 138.5177312231 * log(blue) - 305.0447927307;
        if (blue < 0.0) blue = 0.0;
        if (blue > 255.0) blue = 255.0;
    }

    return CRGB((uint8_t)red, (uint8_t)green, (uint8_t)blue);
}

void LedEngine::renderAstronomyPhase(double phaseAngleDeg, double illumination, bool isWaxing, DayPhaseType dayPhase) {
    CRGB baseColor = kelvinToRGB(_state.kelvin);

    // Дневное авто-диммирование
    float dayMultiplier = 1.0f;
    if (dayPhase == DayPhaseType::DAY) {
        dayMultiplier = 0.05f; // Днем светит как тонкий декоративный контур (5%)
    } else if (dayPhase == DayPhaseType::SUNSET_TWILIGHT || dayPhase == DayPhaseType::DAWN) {
        dayMultiplier = 0.6f;  // В сумерках мягкий свет (60%)
    }

    // Центр освещенной дуги:
    // Растущая (Waxing): 90° (Правая сторона)
    // Убывающая (Waning): 270° (Левая сторона)
    double litCenter = isWaxing ? 90.0 : 270.0;

    // Полуширина световой дуги: от 0° при Новолунии до 180° при Полнолунии
    double halfArcWidth = illumination * 180.0;
    double featherZone = 20.0; // Градусы сглаживания терминатора (антиалиасинг)

    for (int i = 0; i < _state.ledCount; i++) {
        double ledAngle = getLedAngleDeg(i);
        double distFromCenter = getAngularDistance(ledAngle, litCenter);

        float pixelFactor = 0.0f;

        if (distFromCenter <= (halfArcWidth - featherZone)) {
            pixelFactor = 1.0f;
        } else if (distFromCenter >= (halfArcWidth + featherZone)) {
            pixelFactor = 0.015f; // Пепельный свет темной стороны Луны (earthshine)
        } else {
            // Мягкий синусоидальный переход на терминаторе
            float t = (distFromCenter - (halfArcWidth - featherZone)) / (2.0f * featherZone);
            pixelFactor = 0.015f + (0.985f * (0.5f * (1.0f + cos(t * M_PI))));
        }

        pixelFactor *= dayMultiplier;
        _leds[i] = CRGB(
            (uint8_t)(baseColor.r * pixelFactor),
            (uint8_t)(baseColor.g * pixelFactor),
            (uint8_t)(baseColor.b * pixelFactor)
        );
    }
}

void LedEngine::renderBreathe(CRGB baseColor) {
    float wave = (sin(millis() / 1500.0) + 1.0f) * 0.5f; // 0.0 .. 1.0
    float factor = 0.15f + (0.85f * wave);
    for (int i = 0; i < _state.ledCount; i++) {
        _leds[i] = CRGB(
            (uint8_t)(baseColor.r * factor),
            (uint8_t)(baseColor.g * factor),
            (uint8_t)(baseColor.b * factor)
        );
    }
}

void LedEngine::renderEclipse() {
    uint32_t ms = millis();
    for (int i = 0; i < _state.ledCount; i++) {
        // Случайные мерцающие протуберанцы короны через синусоиды
        uint8_t noise = inoise8(i * 35, ms / 12);
        uint8_t flicker = inoise8(i * 50, ms / 6);
        
        CRGB flameColor = ColorFromPalette(HeatColors_p, noise, flicker, LINEARBLEND);
        _leds[i] = flameColor;
    }
}

void LedEngine::renderAurora() {
    uint32_t ms = millis();
    for (int i = 0; i < _state.ledCount; i++) {
        // Две перекрестные синусоидальные волны
        float w1 = sin((i * 12.0f + ms * 0.08f) * M_PI / 180.0f);
        float w2 = sin((i * 18.0f - ms * 0.05f) * M_PI / 180.0f);
        float blend = (w1 + w2) * 0.5f; // -1.0 .. 1.0

        uint8_t g = (uint8_t)(160 + 95 * ((blend + 1.0f) * 0.5f));
        uint8_t b = (uint8_t)(140 + 115 * ((1.0f - blend) * 0.5f));
        uint8_t r = (uint8_t)(30 + 30 * fabs(blend));

        _leds[i] = CRGB(r, g, b);
    }
}

void LedEngine::renderSolid(CRGB color) {
    for (int i = 0; i < _state.ledCount; i++) {
        _leds[i] = color;
    }
}

void LedEngine::update(const MoonTelemetry& moon, const SunTelemetry& sun) {
    if (!_state.power) {
        FastLED.clear(true);
        return;
    }

    CRGB baseColor = kelvinToRGB(_state.kelvin);

    switch (_state.mode) {
        case LAMP_MODE_LIVE_ASTRONOMY:
            renderAstronomyPhase(moon.phaseAngleDeg, moon.illumination, moon.isWaxing, sun.dayPhase);
            break;

        case LAMP_MODE_MANUAL_PHASE: {
            double angle = _state.manualPhaseAngle;
            double illum = (1.0 - cos(angle * M_PI / 180.0)) / 2.0;
            bool isWax = (angle < 180.0);
            renderAstronomyPhase(angle, illum, isWax, DayPhaseType::NIGHT);
            break;
        }

        case LAMP_MODE_BREATHE:
            renderBreathe(baseColor);
            break;

        case LAMP_MODE_ECLIPSE:
            renderEclipse();
            break;

        case LAMP_MODE_AURORA:
            renderAurora();
            break;

        case LAMP_MODE_SOLID:
            renderSolid(baseColor);
            break;
    }

    FastLED.show();
}
