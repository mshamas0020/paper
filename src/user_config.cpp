// user_config.cpp

#include "user_config.hpp"

#include <Preferences.h>



namespace paper {

static Preferences      nvs;

static const char*      KEY_SSID                = "ssid";
static const char*      KEY_PASSWORD            = "pass";
static const char*      KEY_TIME_ZONE           = "tzus";
static const char*      KEY_TIME_ZONE_ENABLED   = "tzon";
static const char*      KEY_GREY_ENABLED        = "grey";

static char             ssid_buf[128]           = {};
static char             password_buf[128]       = {};
static char             time_zone_buf[128]      = {};

UserConfig              user;

static bool get_network_config_required()
{
    return !strlen(ssid_buf) || !strlen(password_buf);
}

static bool get_time_zone_config_required()
{
    return !strlen(time_zone_buf);
}



int user_config_init()
{
    if (!nvs.begin("paper")) {
        Serial.print("Error: NVS\n");
        return -1;
    }

    nvs.getString(KEY_SSID, ssid_buf, sizeof(ssid_buf));
    nvs.getString(KEY_PASSWORD, password_buf, sizeof(password_buf));
    nvs.getString(KEY_TIME_ZONE, time_zone_buf, sizeof(time_zone_buf));

    const bool time_zone_enabled = nvs.getBool(KEY_TIME_ZONE_ENABLED, (int64_t) false);
    const bool grey_enable = nvs.getBool(KEY_GREY_ENABLED, (int64_t) false);

    user.ssid = ssid_buf;
    user.password = password_buf;
    user.time_zone = time_zone_buf;
    user.time_zone_enabled = time_zone_enabled;
    user.grey_enabled = grey_enable;
    user.network_config_required = get_network_config_required();
    user.time_zone_config_required = get_time_zone_config_required();
    return 0;
}

const UserConfig& user_config_get()
{
    return user;
}

int user_config_set_ssid(const char* ssid)
{
    if (!nvs.putString(KEY_SSID, ssid)) {
        Serial.print("Error: NVS\n");
        return -1;
    }

    strcpy(ssid_buf, ssid);
    user.network_config_required = get_network_config_required();
    return 0;
}

int user_config_set_password(const char* password)
{
    if (!nvs.putString(KEY_PASSWORD, password)) {
        Serial.print("Error: NVS\n");
        return -1;
    }

    strcpy(password_buf, password);
    user.network_config_required = get_network_config_required();
    return 0;
}

int user_config_set_time_zone(const char* time_zone)
{    
    if (!nvs.putString(KEY_TIME_ZONE, time_zone)) {
        Serial.print("Error: NVS\n");
        return -1;
    }

    strcpy(time_zone_buf, time_zone);
    user.time_zone_config_required = get_time_zone_config_required();
    return 0;
}

int user_config_set_time_zone_enabled(bool enable)
{
    user.time_zone_enabled = enable;

    if (!nvs.putBool(KEY_TIME_ZONE_ENABLED, user.time_zone_enabled)) {
        Serial.print("Error: NVS\n");
        return -1;
    }

    return 0;
}

int user_config_set_grey_enabled(bool enable)
{
    user.grey_enabled = enable;

    if (!nvs.putBool(KEY_GREY_ENABLED, user.grey_enabled)) {
        Serial.print("Error: NVS\n");
        return -1;
    }

    return 0;
}

int user_config_reset()
{
    memset(ssid_buf, 0, sizeof(ssid_buf));
    memset(password_buf, 0, sizeof(password_buf));
    memset(time_zone_buf, 0, sizeof(time_zone_buf));
    user.time_zone_enabled = false;
    user.grey_enabled = false;
    user.network_config_required = get_network_config_required();
    user.time_zone_config_required = get_time_zone_config_required();
    
    if (!nvs.clear()) {
        Serial.print("Error: NVS\n");
        return -1;
    }

    return 0;
}

} // namespace paper