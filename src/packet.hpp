#ifndef PACKET_HPP_
#define PACKET_HPP_

#include "ethernet.hpp"

/* Ethernet frame types according to RFC 2516 */
#define ETH_PPPOE_DISCOVERY 0x8863
#define ETH_PPPOE_SESSION   0x8864

enum class PPPOE_CODE: uint8_t {
    PADI = 0x09,
    PADO = 0x07,
    PADR = 0x19,
    PADS = 0x65,
    PADT = 0xa7
};

enum class PPPOE_TAG: uint16_t {
    END_OF_LIST = 0x0000,
    SERVICE_NAME = 0x0101,
    AC_NAME = 0x0102,
    HOST_UNIQ = 0x0103,
    AC_COOKIE = 0x0104,
    VENDOR_SPECIFIC = 0x0105,
    RELAY_SESSION_ID = 0x0110,
    SERVICE_NAME_ERROR = 0x0201,
    AC_SYSTEM_ERROR = 0x0202,
    GENERIC_ERROR = 0x0203,
};

enum class LCP_CODE : uint8_t {
    VENDOR_SPECIFIC = 0,
    CONF_REQ = 1,
    CONF_ACK = 2,
    CONF_NAK = 3,
    CONF_REJ = 4,
    TERM_REQ = 5,
    TERM_ACK = 6,
    CODE_REJ = 7,
    PROTO_REJ = 8,
    ECHO_REQ = 9,
	ECHO_REPLY = 10,
	DISCARD_REQ = 11,
	IDENTIFICATION = 12,
	TIME_REMAINING = 13,
};

struct PPPOEDISC_TLV {
    uint16_t type;
    uint16_t length;
    uint8_t *value;
}__attribute__((__packed__));

struct PPPOEDISC_HDR {
    uint32_t type : 4;
    uint32_t version : 4;
    enum PPPOE_CODE code;
    uint16_t session_id;
    uint16_t length;

    uint8_t* getPayload() {
        return reinterpret_cast<uint8_t*>( this ) + sizeof( *this );
    }
}__attribute__((__packed__));
static_assert( sizeof( PPPOEDISC_HDR ) == 6 );

struct PPPOESESSION_HDR {
    uint32_t type : 4;
    uint32_t version : 4;
    enum PPPOE_CODE code;
    uint16_t session_id;
    uint16_t length;
    uint16_t ppp_protocol;

    uint8_t* getPayload() {
        return reinterpret_cast<uint8_t*>( this ) + sizeof( *this );
    }
}__attribute__((__packed__));
static_assert( sizeof( PPPOESESSION_HDR ) == 8 );

struct PPP_LCP {
    LCP_CODE code;
    uint8_t identifier;
    uint16_t length;
}__attribute__((__packed__));

struct Packet {
    ETHERNET_HDR *eth { nullptr };
    PPPOEDISC_HDR *pppoe_discovery { nullptr };
    PPPOESESSION_HDR *pppoe_session { nullptr };
    PPP_LCP *lcp { nullptr };
    
    std::vector<uint8_t> bytes;

    Packet( std::vector<uint8_t> p ):
        bytes( std::move( p ) )
    {}
};

#endif