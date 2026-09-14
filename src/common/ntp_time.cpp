#include "ntp_time.h"
#include "time_formatting.h"
#include <time.h>

bool syncTime() {
    // TZ string encodes US Eastern + DST rules (2nd Sun Mar - 1st Sun Nov),
    // so tm_isdst is computed correctly
    configTzTime("EST5EDT,M3.2.0,M11.1.0", "time.cloudflare.com");

    struct tm timeinfo;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (getLocalTime(&timeinfo, 5000)) return true;
        Serial.println("NTP sync failed, retrying...");
        delay(2000);
    }
    return false;
}

static bool isDaylightSavingNow() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100)) return false;  // fallback: standard time
    return timeinfo.tm_isdst > 0;
}

String getCurrentDate() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100)) return "0000-00-00";
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
              timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    return String(buf);
}

String convertUtcToEst(const String& timeStrHHMM) {
    int offset = isDaylightSavingNow() ? -4 : -5;
    return convertUtcToEstWithOffset(timeStrHHMM, offset);
}
