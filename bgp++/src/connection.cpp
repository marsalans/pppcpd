#include "main.hpp"

void bgp_connection::start() {
    auto const &endpoint = sock.remote_endpoint();
    log( "Incoming connection: "s + endpoint.address().to_string() + " "s + std::to_string( endpoint.port() ) );
    do_read();
}

void bgp_connection::on_receive( error_code ec, std::size_t length ) {
    if( ec ) {
        log( "Error on receiving data: "s + ec.message() );
        return;
    }
    log( "Received message of size: "s + std::to_string( length ) );
    bgp_packet pkt { buffer.begin(), length };
    auto bgp_header = pkt.get_header();
    if( std::any_of( bgp_header->marker.begin(), bgp_header->marker.end(), []( uint8_t el ) { return el != 0xFF; } ) ) {
        log( "Wrong BGP marker in header!" );
        return;
    }
    switch( bgp_header->type ) {
    case bgp_type::OPEN:
        log( "OPEN message" );
        process_open( pkt );
        break;
    case bgp_type::KEEPALIVE:
        log( "KEEPALIVE message" );
        break;
    case bgp_type::UPDATE:
        log( "UPDATE message" );
        break;
    case bgp_type::NOTIFICATION:
        log( "NOTIFICATION message" );
        break;
    case bgp_type::ROUTE_REFRESH:
        log( "ROUTE_REFRESH message" );
        break;
    }
    do_read();
}

void bgp_connection::do_read() {
    sock.async_receive( boost::asio::buffer( buffer ), std::bind( &bgp_connection::on_receive, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
}

void bgp_connection::process_open( bgp_packet &pkt ) {
    auto open = pkt.get_open();

    log( "BGP version: "s + std::to_string( open->version ) );
    log( "Router ID: "s + address_v4( bswap32( open->bgp_id ) ).to_string() );
    log( "Hold time: "s + std::to_string( bswap16( open->hold_time ) ) );
}