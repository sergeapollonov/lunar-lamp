#pragma once

#include <Arduino.h>
#include <time.h>

enum class MoonPhaseType {
    NEW_MOON,           // Новолуние
    WAXING_CRESCENT,    // Растущий серп
    FIRST_QUARTER,      // Первая четверть
    WAXING_GIBBOUS,     // Растущая Луна
    FULL_MOON,          // Полнолуние
    WANING_GIBBOUS,     // Убывающая Луна
    THIRD_QUARTER,      // Последняя четверть
    WANING_CRESCENT     // Убывающий серп
};

enum class DayPhaseType {
    DAY,                // День (лампа выключена или 2-3% декор)
    SUNSET_TWILIGHT,    // Сумерки/закат (плавное разгорание)
    NIGHT,              // Ночь (активная подсветка заданной яркости)
    DAWN                // Рассвет (плавное угасание)
};

struct MoonTelemetry {
    double phaseAngleDeg;       // Фазовый угол: 0°..360° (0=Новолуние, 180=Полнолуние)
    double illumination;        // Освещенность диска: 0.0 .. 1.0 (0% .. 100%)
    double ageDays;             // Возраст Луны в днях (0.0 .. 29.53)
    bool isWaxing;              // true = растущая (свет справа), false = убывающая (свет слева)
    MoonPhaseType phaseType;    // Тип фазы (8 градаций)
    const char* phaseNameEn;    // Название на английском
    const char* phaseNameRu;    // Название на русском
};

struct SunTelemetry {
    int sunriseMinutes;         // Минуты от полуночи (локальное время)
    int sunsetMinutes;          // Минуты от полуночи (локальное время)
    int sunriseHour;
    int sunriseMinute;
    int sunsetHour;
    int sunsetMinute;
    DayPhaseType dayPhase;      // Текущая фаза дня
    const char* dayPhaseNameRu;
};

class AstronomyEngine {
public:
    AstronomyEngine(double latitude = 52.2297, double longitude = 21.0122); // По умолчанию Варшава

    void setCoordinates(double latitude, double longitude);
    
    // Расчет параметров Луны для заданного UNIX timestamp (UTC)
    MoonTelemetry calculateMoon(time_t utcEpoch);

    // Расчет восхода и заката для заданного локального времени (struct tm)
    SunTelemetry calculateSun(const struct tm& localTime, int timezoneOffsetMinutes);

    // ASCII арт текущей фазы для Serial Monitor
    const char* getAsciiMoon(MoonPhaseType phase);

private:
    double _lat;
    double _lon;

    double toJulianDate(time_t epoch);
    double degToRad(double deg);
    double radToDeg(double rad);
};
