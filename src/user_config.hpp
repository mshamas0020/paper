// user_config.hpp

#ifndef __PAPER_USER_CONFIG_HPP__
#define __PAPER_USER_CONFIG_HPP__



namespace paper {

struct UserConfig
{
    const char* ssid;
    const char* password;
    const char* time_zone;
    bool        time_zone_enabled;
    bool        grey_enabled;

    bool        network_config_required;
    bool        time_zone_config_required;
};

int user_config_init();
int user_config_reset();

const UserConfig& user_config_get();

int user_config_set_ssid(const char* ssid);
int user_config_set_password(const char* password);
int user_config_set_time_zone(const char* time_zone);
int user_config_set_time_zone_enabled(bool enable);
int user_config_set_grey_enabled(bool enable);

} // namespace paper



#endif // __PAPER_USER_CONFIG_HPP__