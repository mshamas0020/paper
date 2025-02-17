// term.cpp

#include "term.hpp"

#include "network.hpp"
#include "posix_tz_db.h"
#include "sync.hpp"
#include "user_config.hpp"
#include "utils.hpp"

#include <Preferences.h>



namespace paper {

static void cmd_help(const char* arg)
{
    Serial.print(
        "Commands\n\n"
        "    help                     : Show this help menu.\n"
        "    set ssid [ssid]          : Set WiFi SSID.\n"
        "    set password [password]  : Set WiFi password.\n"
        "    set timezone [time zone] : Set time zone code manually.\n"
        "    set geolocation [on|off] : Enable / disable geolocation.\n"
        "                             : If enabled, overrides time zone code.\n"
        "    set grey [on|off]        : Enable / disable 4-color grey display.\n"
        "    list timezones           : List all valid time zone codes.\n"
        "    network                  : Test network connection.\n"
        "    reset                    : Clear user configuration.\n"
        "    status                   : Print current user configuration.\n"
        "    sync                     : Connect to network and sync local time.\n"
        "    time                     : Print local time.\n"
        "    exit                     : Exit terminal.\n"
    );
}

static void cmd_set_ssid(const char* arg)
{
    if (!strlen(arg)) {
        Serial.print("Invalid argument.\n");
        return;
    }

    if (user_config_set_ssid(arg))
        return;

    Serial.print("Done.\n");
}

static void cmd_set_password(const char* arg)
{
    if (!strlen(arg)) {
        Serial.print("Invalid argument.\n");
        return;
    }
    
    if (user_config_set_password(arg))
        return;

    Serial.print("Done.\n");
}

static void cmd_set_timezone(const char* arg)
{    
    if (!strlen(arg)) {
        Serial.print("Invalid argument.\n");
        return;
    }

    if (user_config_set_time_zone(arg))
        return;

    Serial.print("Done.\n");

    if (!user_config_get().time_zone_enabled)
        Serial.print(
            "NOTE: Geolocation is enabled and will override the set time zone. "
            "To disable, enter 'set geolocation off'.\n"
        );
}

static void cmd_set_geolocation(const char* arg)
{
    bool on;
    if (strcasecmp(arg, "on") == 0) {
        on = true;
    } else if (strcasecmp(arg, "off") == 0) {
        on = false;
    } else {
        Serial.print("Invalid value. Must be [on|off].");
        return;
    }

    if (user_config_set_time_zone_enabled(!on))
        return;

    Serial.print("Done.\n");

    if (!on && user_config_get().time_zone_config_required)
        Serial.print("NOTE: Time zone not set. May not be able to sync.\n");
}

static void cmd_set_grey(const char* arg)
{
    bool on;
    if (strcasecmp(arg, "on") == 0) {
        on = true;
    } else if (strcasecmp(arg, "off") == 0) {
        on = false;
    } else {
        Serial.print("Invalid value. Must be [on|off].");
        return;
    }

    if (user_config_set_grey_enabled(on))
        return;

    Serial.print("Done.\n");
}

static void cmd_network(const char* arg)
{
    const UserConfig& user = user_config_get();

    if (user.network_config_required) {
        Serial.print("WiFi SSID and/or password is not set.\n");
        return;
    }

    if (network_connect(user.ssid, user.password))
        return;

    Serial.print("Success.\n");
    network_disconnect();
}

static void cmd_reset(const char* arg)
{
    Serial.print("Erase user configuration? This cannot be undone. (Y/n)\n");

    bool confirmed = false;
    while (!confirmed) {
        while (Serial.available() > 0) {
            int c = Serial.read();
            if (c == 'n' || c == 'N')
                return;
            if (c == 'y' || c == 'Y') {
                confirmed = true;
                break;
            }
        }
    }

    if (user_config_reset())
        return;

    Serial.print("Done.\n");
}

static void cmd_status(const char* arg)
{
    const UserConfig& user = user_config_get();

    Serial.print("User Configuration\n\n");
    Serial.printf("    WiFi SSID       : %s\n",
        strlen(user.ssid) ? user.ssid : "(not found)");
    {
        Serial.print("    WiFi Password   : ");
        const int password_len = strlen(user.password);
        if (password_len) {
            for (int i = 0; i < password_len; i++)
                Serial.print("*");
        } else {
            Serial.print("(not found)");
        }
        Serial.print("\n");
    }
    Serial.printf("    Time Zone       : %s\n",
        strlen(user.time_zone) ? user.time_zone : "(not found)");
    Serial.printf("    Geolocation     : %s\n",
        !user.time_zone_enabled ? "Enabled" : "Disabled" );
        
    Serial.printf("    Display         : %s\n",
        user.grey_enabled ? "4-color" : "2-color" );
}

static void cmd_sync(const char* arg)
{   
    const UserConfig& user = user_config_get();

    if (user.network_config_required) {
        Serial.print("WiFi SSID and/or password is not set.\n");
        return;
    }

    int rc = paper::sync_ntp(user);
    if (rc) 
        Serial.printf("Error: Sync failed (%d).\n", rc);
}

static void cmd_time(const char* arg)
{
    struct tm time_info = get_time_info();
    Serial.print(&time_info, "%A, %B %d %Y %H:%M:%S\n");
}

static void cmd_list_timezones(const char* arg)
{
    Serial.print("Region                          Code\n\n");
    for (int i = 0; i < sizeof(posix_tz_db) / sizeof(posix_tz_db[0]); i++)
        Serial.printf("%-30s  %s\n",posix_tz_db[i].key, posix_tz_db[i].value);
}

const struct { const char* cmd; void (*fn)(const char* arg); } BOOK[] = {
    { "help",               cmd_help },
    { "set ssid",           cmd_set_ssid },
    { "set password",       cmd_set_password },
    { "set timezone",       cmd_set_timezone },
    { "set geolocation",    cmd_set_geolocation },
    { "set grey",           cmd_set_grey },
    { "list timezones",     cmd_list_timezones },
    { "network",            cmd_network },
    { "reset",              cmd_reset },
    { "status",             cmd_status },
    { "sync",               cmd_sync },
    { "time",               cmd_time },
};

static int term_read(char* buf, int buf_size)
{
    int idx = 0;

    while (true) {
        while (Serial.available() > 0) {
            int c = Serial.read();

            if (c == '\n' || c == '\r') {
                // return
                Serial.print("\r\n");
                return idx;
            } else if (c == '\b') {
                // backspace
                if (idx > 0) {
                    Serial.print("\b\033[K");
                    buf[idx] = '\0';
                    idx--;
                }
                continue;
            } else if (c < 32 || c > 126) {
                continue;
            }

            Serial.printf("%c", (char) c);
            buf[idx] = (char) c;

            idx ++;

            if (idx >= buf_size)
                return idx;
        }
    }
}

bool term_enter()
{
    Serial.print("Press any key to enter the terminal.\n");

    Serial.setTimeout(30 * 1000); // 30s
    char _;
    const int n_bytes = Serial.readBytes(&_, 1);

    if (n_bytes == 0) {
        Serial.print("Timed out.\n");
        return false;
    }
    
    Serial.print("\033[2m");
    cmd_help(nullptr);
    Serial.print("\033[0m");

    char cmd_buf[256];
    while (true) {
        memset(cmd_buf, 0, sizeof(cmd_buf));

        Serial.print(">>> ");
        const int n_bytes = term_read(cmd_buf, sizeof(cmd_buf) - 2);

        if (strcmp("exit", cmd_buf) == 0)
            return true;

        for (int i = 0; i < sizeof(BOOK) / sizeof(BOOK[0]); i++) {
            const char* cmd = BOOK[i].cmd;
            const int cmd_len = strlen(cmd);
            if (strncasecmp(cmd, cmd_buf, cmd_len) == 0) {
                const char* arg = &cmd_buf[cmd_len + 1];

                Serial.print("\033[2m");
                BOOK[i].fn(arg);
                Serial.print("\033[0m");
            }
        }
    }

    return true;
}

} // namespace paper