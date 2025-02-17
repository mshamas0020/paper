// sync.cpp

#include "sync.hpp"

#include "network.hpp"
#include "utils.hpp"

#include <Arduino.h>
#include "esp32-hal.h"
#include "esp_sntp.h"



namespace paper {

static const char* NTP_SERVER1      = "pool.ntp.org";
static const char* NTP_SERVER2      = "time.nist.gov";

static constexpr int UTC_DST_OFFSET_INVALID = INT_MAX;

RTC_DATA_ATTR int rtc_mem_utc_dst_offset = UTC_DST_OFFSET_INVALID;


static int ntp_config_api()
{
    const int utc_dst_offset = network_api_utc_dst_offset();
    if (utc_dst_offset == UTC_DST_OFFSET_INVALID)
        return -1;

    rtc_mem_utc_dst_offset = utc_dst_offset;

    configTime(utc_dst_offset, 0, NTP_SERVER1, NTP_SERVER2);
    return 0;
}

static int ntp_config_tz(const char* tz)
{
    Serial.printf("SYNC    > Setting user time zone '%s'.\n", tz);
    configTzTime(tz, NTP_SERVER1, NTP_SERVER2);
    return 0;
}

static time_t ntp_wait_sync()
{
    static constexpr int TIMEOUT_MS = 10000;

    time_t sync_time;
    uint32_t start = millis();

    Serial.print("SYNC    > NTP configuring local time.");

    do {
        sync_time = get_time();

        if (millis() - start >= TIMEOUT_MS) {
            Serial.print("\n");
            Serial.print("Error: NTP could not sync time.\n");
            return 0;
        }

        delay(500);
        Serial.print(".");
    } while (!is_time_valid(sync_time));

    Serial.print(" done.\n");

    struct tm sync_time_info = to_time_info(sync_time);
    Serial.print(&sync_time_info, "SYNC    > %A, %B %d %Y %H:%M:%S\n");

    return sync_time;
}



int sync_ntp(const UserConfig& config)
{
    const bool use_time_zone = (config.time_zone_enabled && !config.time_zone_config_required);

    uint32_t start = millis();
    time_t prev_time = get_time();
    time_t sync_time;

    int rc = network_connect(config.ssid, config.password);
    if (rc) goto err_exit;

    if (use_time_zone) {
        rc = ntp_config_tz(config.time_zone);
        if (rc) goto err_exit;
    } else {
        rc = ntp_config_api();
        if (rc) goto err_exit;
    }

    sync_time = ntp_wait_sync();
    if (sync_time == 0) {
        rc = -1;
        goto err_exit;
    }
    
    if (is_time_valid(prev_time)) {
        const int sync_duration = (int) roundf((float) (millis() - start) / 1000.f);
        const time_t expected_time = prev_time + (time_t) sync_duration;
        const int drift = (int) (sync_time - expected_time);
        Serial.printf("SYNC    > Drift: %d s\n", drift);
    }

err_exit:
    network_disconnect();
    return rc;
}

int sync_rtc_mem(const UserConfig& config)
{    
    const bool use_time_zone = (config.time_zone_enabled && !config.time_zone_config_required);

    char tz[64] = {};

    if (use_time_zone) {
        Serial.printf("SYNC    > Setting user time zone '%s'.\n", tz);
        strcpy(tz, config.time_zone);
    } else {
        if (rtc_mem_utc_dst_offset == UTC_DST_OFFSET_INVALID) {
            Serial.print("Error: Stored UTC offset is invalid\n");
            return -1;
        }

        const int offset = - rtc_mem_utc_dst_offset;

        Serial.printf("SYNC    > Recalled UTC offset (%d s).\n", rtc_mem_utc_dst_offset);

        if (offset % 3600){
            sprintf(tz, "UTC%ld:%02u:%02uDST0", offset / 3600, abs((offset % 3600) / 60), abs(offset % 60));
        } else {
            sprintf(tz, "UTC%ldDST0", offset / 3600);
        }
    }

    setenv("TZ", tz, 1);
    tzset();

    return 0;
}

} // namespace paper