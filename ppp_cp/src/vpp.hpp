#ifndef VPP_API_HPP_
#define VPP_API_HPP_

#include "vapi/vapi.hpp"
#include "vapi/vpe.api.vapi.hpp"

#include "vapi/pppoe.api.vapi.hpp"

class VPPAPI {
public:
    VPPAPI( std::unique_ptr<Logger> &l );

    ~VPPAPI();

    bool add_pppoe_session( uint32_t ip_address, uint16_t session_id, std::array<uint8_t,6> mac, bool is_add = true );
private:
    std::unique_ptr<Logger> &logger;
    vapi::Connection con;
};

#endif