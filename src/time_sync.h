#pragma once

#include <Arduino.h>
#include <time.h>

class TimeSync {
public:
    TimeSync(const char* ntpServer = "pool.ntp.org", 
             const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3"); // Польша / Ц. Европа

    void begin();
    bool isSynchronized();
    time_t getUtcEpoch();
    struct tm getLocalTime();
    int getTimezoneOffsetMinutes();
    String getFormattedLocalTime();

private:
    const char* _ntpServer;
    const char* _timezone;
    bool _isSyncd;
};
