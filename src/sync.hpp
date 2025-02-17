// sync.hpp

#ifndef __PAPER_SYNC_HPP__
#define __PAPER_SYNC_HPP__

#include "user_config.hpp"

#include <ctime>



namespace paper {

int sync_ntp(const UserConfig& config);

int sync_rtc_mem(const UserConfig& config);

} // namespace paper



#endif // __PAPER_SYNC_HPP__