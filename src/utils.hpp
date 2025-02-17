// utils.hpp

#ifndef __PAPER_UTILS_HPP__
#define __PAPER_UTILS_HPP__

#include <ctime>



namespace paper {

inline time_t get_time()
{
    return time(nullptr);
}

inline struct tm to_time_info(time_t t)
{
    struct tm info;
    localtime_r(&t, &info);
    return info;
}

inline struct tm get_time_info()
{
    return to_time_info(get_time());
}

inline bool is_time_info_valid(struct tm t)
{
    return (t.tm_year >= (2025 - 1900));
}

inline bool is_time_valid(time_t t)
{
    return is_time_info_valid(to_time_info(t));
}

inline bool get_time_valid()
{
    return is_time_info_valid(get_time_info());
}

} // namespace paper



#endif // __PAPER_UTILS_HPP__