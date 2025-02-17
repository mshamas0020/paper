// main.cpp

#include "draw.hpp"
#include "sync.hpp"
#include "term.hpp"
#include "user_config.hpp"
#include "utils.hpp"

#include <Arduino.h>

    // time zones
    // "PST8PDT,M3.2.0,M11.1.0", // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv


static constexpr int    SYNC_INTERVAL   = 24 * 3600;    // 24 hours
static constexpr int    SYNC_DAILY_TIME = 2 * 60;       // 2 AM, minutes from midnight

RTC_DATA_ATTR int   boot_count                     = 0;
RTC_DATA_ATTR int   boots_since_last_sync_attempt  = 0;
RTC_DATA_ATTR bool  sync_failed                    = false;



static bool get_sync_required(bool first_boot)
{
    if (first_boot) {
        Serial.print("MAIN    > Sync required: First boot.\n");
        return true;
    }

    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER) {
        Serial.print("MAIN    > Sync required: Unknown wakeup reason.\n");
        return true;
    }

    if (!paper::get_time_valid()) {
        Serial.print("MAIN    > Sync required: Time invalid.\n");
        return true;
    }

    if (boots_since_last_sync_attempt >= SYNC_INTERVAL) {
        Serial.print("MAIN    > Sync required: Sync interval reached.\n");
        return true;
    }

    const struct tm time_info = paper::get_time_info();
    const int minute = time_info.tm_hour * 60 + time_info.tm_min;
    if (minute == SYNC_DAILY_TIME) {
        Serial.print("MAIN    > Sync required: Sync time reached.\n");
        return true;
    }

    return false;
}

static char time_str[32] = {};
static char date_str[32] = {};

static paper::DrawInfo make_draw_info(const struct tm& time_info, const paper::UserConfig& user)
{
    const char* months[12] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    // time string
    snprintf(time_str, sizeof(time_str), "%d:%02d", time_info.tm_hour % 12, time_info.tm_min);

    // date string
    snprintf(date_str, sizeof(date_str), "%s %d", months[time_info.tm_mon], (int) time_info.tm_mday);

    paper::DrawInfo draw_info;
    draw_info.enable_2bpp = user.grey_enabled;
    draw_info.network_error = sync_failed || user.network_config_required;
    draw_info.time_str = time_str;
    draw_info.date_str = date_str;
    return draw_info;
}

static void run_terminal()
{
    const bool entered = paper::term_enter();

    if (!entered)
        return;

    const paper::UserConfig& user = paper::user_config_get();
    if (!user.network_config_required) {
        int rc = paper::sync_ntp(user);
        if (rc) 
            Serial.printf("Error: Sync failed (%d).\n", rc);

        sync_failed = (rc != 0);
    }

    const struct tm time_info = paper::get_time_info();
    const paper::DrawInfo draw_info = make_draw_info(time_info, user);

    int rc = paper::draw(draw_info);
    if (rc)
        Serial.printf("Error: Draw failed (%d).\n", rc);
}

static void led_blink()
{
    pinMode(LED_BUILTIN, OUTPUT);

    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
    }
}

void setup()
{
    const bool first_boot = (boot_count == 0);
    boot_count++;

    delay(1000);
    Serial.begin(115200);

    if (first_boot) {
        led_blink();
        Serial.print("\n\n--- PAPER v0.0 ---\n");
    }

    Serial.printf("MAIN    > Boot Count: %d\n", boot_count);

    const bool sync_required = get_sync_required(first_boot);

    paper::user_config_init();
    const paper::UserConfig& user = paper::user_config_get();

    if (user.network_config_required) {
        Serial.print("Error: Network config required.\n");
        paper::sync_rtc_mem(user);
    } else if (sync_required) {
        boots_since_last_sync_attempt = 0;

        int rc = paper::sync_ntp(user);
        if (rc) 
            Serial.printf("Error: Sync failed (%d).\n", rc);

        sync_failed = (rc != 0);
        
    } else {
        paper::sync_rtc_mem(user);
    }

    int draw_count = 0;
    while (true) {
        const struct tm time_info = paper::get_time_info();
        const paper::DrawInfo draw_info = make_draw_info(time_info, user);

        int rc = paper::draw(draw_info);
        if (rc) {
            Serial.printf("FATAL: Draw failed (%d).\n", rc);
            return;
        }

        // this should only run once or twice (if we boot near a minute crossing)
        draw_count++;
        if (draw_count > 2) {
            Serial.print("FATAL: Max draw count reached.\n");
            return;
        }

        const struct tm next_time_info = paper::get_time_info();
        if (next_time_info.tm_min == time_info.tm_min)
            break; // display matches current time
    }

    if (first_boot)
        run_terminal();

    // go to sleep
    {
        const int s_until_boot = 60 - paper::get_time_info().tm_sec;
        
        Serial.printf("MAIN    > Sleeping (%d s).\n\n", s_until_boot);
        Serial.flush();
        esp_sleep_enable_timer_wakeup(s_until_boot * 1000000 /* us */);
        esp_deep_sleep_start();
        delay(1000);
    }
}

void loop()
{
    // should not reach here

    delay(2000);
    Serial.print("Unknown Fatal Error\n");
}