#include "time_sync.h"
#include <esp_sntp.h>

TimeSync::TimeSync(const char* ntpServer, const char* timezone)
    : _ntpServer(ntpServer), _timezone(timezone), _isSyncd(false) {}

void TimeSync::begin() {
    configTzTime(_timezone, _ntpServer, "time.google.com", "time.cloudflare.com");
}

bool TimeSync::isSynchronized() {
    time_t now = time(nullptr);
    // Год 2024+ означает успешную синхронизацию
    if (now > 1704067200) { 
        _isSyncd = true;
    }
    return _isSyncd;
}

time_t TimeSync::getUtcEpoch() {
    return time(nullptr);
}

struct tm TimeSync::getLocalTime() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    return timeinfo;
}

int TimeSync::getTimezoneOffsetMinutes() {
    time_t now = time(nullptr);
    struct tm localTm, gmtTm;
    localtime_r(&now, &localTm);
    gmtime_r(&now, &gmtTm);

    time_t localSec = mktime(&localTm);
    time_t gmtSec = mktime(&gmtTm);
    return (int)((localSec - gmtSec) / 60);
}

String TimeSync::getFormattedLocalTime() {
    struct tm ti = getLocalTime();
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
             ti.tm_hour, ti.tm_min, ti.tm_sec);
    return String(buf);
}
