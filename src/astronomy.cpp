#include "astronomy.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Известное новолуние: 6 января 2000 года, 18:14 UTC (Unix: 947182440)
static const time_t REFERENCE_NEW_MOON = 947182440;
static const double SYNODIC_MONTH_SEC = 29.53058867 * 86400.0;
static const double SYNODIC_MONTH_DAYS = 29.53058867;

AstronomyEngine::AstronomyEngine(double latitude, double longitude)
    : _lat(latitude), _lon(longitude) {}

void AstronomyEngine::setCoordinates(double latitude, double longitude) {
    _lat = latitude;
    _lon = longitude;
}

double AstronomyEngine::degToRad(double deg) {
    return deg * (M_PI / 180.0);
}

double AstronomyEngine::radToDeg(double rad) {
    return rad * (180.0 / M_PI);
}

double AstronomyEngine::toJulianDate(time_t epoch) {
    return (epoch / 86400.0) + 2440587.5;
}

MoonTelemetry AstronomyEngine::calculateMoon(time_t utcEpoch) {
    MoonTelemetry tele;

    double diffSec = difftime(utcEpoch, REFERENCE_NEW_MOON);
    double cycles = diffSec / SYNODIC_MONTH_SEC;
    double currentCycleProgress = cycles - floor(cycles); // 0.0 .. 1.0

    tele.ageDays = currentCycleProgress * SYNODIC_MONTH_DAYS;
    tele.phaseAngleDeg = currentCycleProgress * 360.0;

    // Освещенность k = (1 - cos(theta)) / 2
    tele.illumination = (1.0 - cos(degToRad(tele.phaseAngleDeg))) / 2.0;
    tele.isWaxing = (tele.phaseAngleDeg < 180.0);

    // Классификация по 8 секторам (шаг 45° с центрами в 0°, 45°, 90°, 135°, 180°...)
    double normalizedAngle = tele.phaseAngleDeg;
    if (normalizedAngle >= 337.5 || normalizedAngle < 22.5) {
        tele.phaseType = MoonPhaseType::NEW_MOON;
        tele.phaseNameEn = "New Moon";
        tele.phaseNameRu = "Новолуние";
    } else if (normalizedAngle < 67.5) {
        tele.phaseType = MoonPhaseType::WAXING_CRESCENT;
        tele.phaseNameEn = "Waxing Crescent";
        tele.phaseNameRu = "Растущий серп";
    } else if (normalizedAngle < 112.5) {
        tele.phaseType = MoonPhaseType::FIRST_QUARTER;
        tele.phaseNameEn = "First Quarter";
        tele.phaseNameRu = "Первая четверть";
    } else if (normalizedAngle < 157.5) {
        tele.phaseType = MoonPhaseType::WAXING_GIBBOUS;
        tele.phaseNameEn = "Waxing Gibbous";
        tele.phaseNameRu = "Растущая Луна";
    } else if (normalizedAngle < 202.5) {
        tele.phaseType = MoonPhaseType::FULL_MOON;
        tele.phaseNameEn = "Full Moon";
        tele.phaseNameRu = "Полнолуние";
    } else if (normalizedAngle < 247.5) {
        tele.phaseType = MoonPhaseType::WANING_GIBBOUS;
        tele.phaseNameEn = "Waning Gibbous";
        tele.phaseNameRu = "Убывающая Луна";
    } else if (normalizedAngle < 292.5) {
        tele.phaseType = MoonPhaseType::THIRD_QUARTER;
        tele.phaseNameEn = "Third Quarter";
        tele.phaseNameRu = "Последняя четверть";
    } else {
        tele.phaseType = MoonPhaseType::WANING_CRESCENT;
        tele.phaseNameEn = "Waning Crescent";
        tele.phaseNameRu = "Убывающий серп";
    }

    return tele;
}

SunTelemetry AstronomyEngine::calculateSun(const struct tm& localTime, int timezoneOffsetMinutes) {
    SunTelemetry sun;

    // Расчет дня года (N)
    int dayOfYear = localTime.tm_yday + 1;

    // Стандартный алгоритм NOAA Solar Calculations
    double lngHour = _lon / 15.0;

    auto computeRiseSet = [&](bool isSunrise) -> double {
        double t = dayOfYear + ((isSunrise ? 6.0 : 18.0) - lngHour) / 24.0;
        double M = (0.9856 * t) - 3.289;
        double L = M + (1.916 * sin(degToRad(M))) + (0.020 * sin(degToRad(2 * M))) + 282.634;
        L = fmod(L, 360.0);
        if (L < 0) L += 360.0;

        double RA = radToDeg(atan(0.91764 * tan(degToRad(L))));
        RA = fmod(RA, 360.0);
        if (RA < 0) RA += 360.0;

        double Lquadrant = floor(L / 90.0) * 90.0;
        double RAquadrant = floor(RA / 90.0) * 90.0;
        RA = RA + (Lquadrant - RAquadrant);
        RA = RA / 15.0; // перевод в часы

        double sinDec = 0.39782 * sin(degToRad(L));
        double cosDec = cos(asin(sinDec));

        // Зенит 90°50' = 90.833° для восхода/заката
        double cosH = (cos(degToRad(90.833)) - (sinDec * sin(degToRad(_lat)))) / (cosDec * cos(degToRad(_lat)));

        if (cosH > 1.0) return -1.0; // Полярная ночь
        if (cosH < -1.0) return -2.0; // Полярный день

        double H = isSunrise ? (360.0 - radToDeg(acos(cosH))) : radToDeg(acos(cosH));
        H = H / 15.0;

        double T = H + RA - (0.06571 * t) - 6.622;
        double UT = fmod(T - lngHour, 24.0);
        if (UT < 0) UT += 24.0;

        // Перевод в локальные минуты от полуночи
        double localMinutes = (UT * 60.0) + timezoneOffsetMinutes;
        localMinutes = fmod(localMinutes, 1440.0);
        if (localMinutes < 0) localMinutes += 1440.0;

        return localMinutes;
    };

    double riseMins = computeRiseSet(true);
    double setMins = computeRiseSet(false);

    sun.sunriseMinutes = (int)riseMins;
    sun.sunsetMinutes = (int)setMins;
    sun.sunriseHour = sun.sunriseMinutes / 60;
    sun.sunriseMinute = sun.sunriseMinutes % 60;
    sun.sunsetHour = sun.sunsetMinutes / 60;
    sun.sunsetMinute = sun.sunsetMinutes % 60;

    int currentMinutes = localTime.tm_hour * 60 + localTime.tm_min;

    // Определение фазы дня:
    // Сумерки заката: за 30 минут до заката и 45 минут после
    // Рассвет: за 45 минут до восхода и 30 минут после
    if (currentMinutes >= (sun.sunsetMinutes - 30) && currentMinutes <= (sun.sunsetMinutes + 45)) {
        sun.dayPhase = DayPhaseType::SUNSET_TWILIGHT;
        sun.dayPhaseNameRu = "Сумерки (Закат)";
    } else if (currentMinutes >= (sun.sunriseMinutes - 45) && currentMinutes <= (sun.sunriseMinutes + 30)) {
        sun.dayPhase = DayPhaseType::DAWN;
        sun.dayPhaseNameRu = "Рассвет";
    } else if (currentMinutes > (sun.sunriseMinutes + 30) && currentMinutes < (sun.sunsetMinutes - 30)) {
        sun.dayPhase = DayPhaseType::DAY;
        sun.dayPhaseNameRu = "День";
    } else {
        sun.dayPhase = DayPhaseType::NIGHT;
        sun.dayPhaseNameRu = "Ночь";
    }

    return sun;
}

const char* AstronomyEngine::getAsciiMoon(MoonPhaseType phase) {
    switch (phase) {
        case MoonPhaseType::NEW_MOON:        return "🌑 (Новолуние)";
        case MoonPhaseType::WAXING_CRESCENT: return "🌒 (Растущий серп)";
        case MoonPhaseType::FIRST_QUARTER:   return "🌓 (Первая четверть)";
        case MoonPhaseType::WAXING_GIBBOUS:  return "🌔 (Растущая Луна)";
        case MoonPhaseType::FULL_MOON:       return "🌕 (Полнолуние)";
        case MoonPhaseType::WANING_GIBBOUS:  return "🌖 (Убывающая Луна)";
        case MoonPhaseType::THIRD_QUARTER:   return "🌗 (Последняя четверть)";
        case MoonPhaseType::WANING_CRESCENT: return "🌘 (Убывающий серп)";
        default:                             return "🌕 (Луна)";
    }
}
