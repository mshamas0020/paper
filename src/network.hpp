// network.hpp

#ifndef __PAPER_NETWORK_HPP__
#define __PAPER_NETWORK_HPP__

#include <WString.h>



namespace paper {

int network_connect(const char* ssid, const char* password);
void network_disconnect();
String network_http_request(const char* request);
int network_api_utc_dst_offset();

} // namespace paper



#endif // __PAPER_NETWORK_HPP__