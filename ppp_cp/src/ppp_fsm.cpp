#include "main.hpp"

extern std::shared_ptr<PPPOERuntime> runtime;
extern PPPOEQ ppp_outcoming;

FSM_RET PPP_FSM::receive( std::vector<uint8_t> &inPkt ) {
    log( "receive pkt in state: " + std::to_string( state ) );
    PPPOESESSION_HDR *pppoe = reinterpret_cast<PPPOESESSION_HDR*>( inPkt.data() );
    PPP_LCP *lcp = reinterpret_cast<PPP_LCP*>( pppoe->getPayload() );

    if( lcp == nullptr ) {
        return { PPP_FSM_ACTION::NONE, "LCP pointer is incorrect" };
    }

    if( state == PPP_FSM_STATE::Initial || 
        state == PPP_FSM_STATE::Starting ) {
            return { PPP_FSM_ACTION::NONE, "Received packet in invalid state: "s + std::to_string( state ) };
    }
    
    switch( lcp->code ) {
    case LCP_CODE::CONF_REQ:
        return recv_conf_req( inPkt );
        break;
    case LCP_CODE::CONF_ACK:
        return recv_conf_ack( inPkt );
	    break;
    
    case LCP_CODE::CONF_NAK:
    case LCP_CODE::CONF_REJ:
	    //fsm_rconfnakrej(f, code, id, inp, len);
	    break;
    
    case LCP_CODE::TERM_REQ:
	    return recv_term_req( inPkt );
	    break;
    
    case LCP_CODE::TERM_ACK:
	    //fsm_rtermack(f);
	    break; 
    
    case LCP_CODE::CODE_REJ:
        //fsm_rcoderej
        break;
    case LCP_CODE::ECHO_REQ:
        return send_echo_rep( inPkt );
    default:
        //send CODEREJ
        break;
    }
    log( "FSM state: " + std::to_string( state ) );

    return { PPP_FSM_ACTION::NONE, "" };
}

FSM_RET PPP_FSM::recv_conf_req( std::vector<uint8_t> &inPkt ) {
    log( "recv_conf_req current state: " + std::to_string( state ) );
    switch( state ){
    case PPP_FSM_STATE::Closing:
    case PPP_FSM_STATE::Stopping:
        return { PPP_FSM_ACTION::NONE, "We're stopping or closing right now" };
    case PPP_FSM_STATE::Closed:
        // send TERM ACK
        return { PPP_FSM_ACTION::NONE, "Receive conf req in closed state" };
    case PPP_FSM_STATE::Opened:
        // Restart connection
        layer_down();
        send_conf_req();
        state = PPP_FSM_STATE::Req_Sent;
        return { PPP_FSM_ACTION::LAYER_DOWN, "" };
        break;
    case PPP_FSM_STATE::Stopped:
        send_conf_req();
        state = PPP_FSM_STATE::Req_Sent;
        break;
    default:
        break;
    }

    return check_conf( inPkt );
}

void PPP_FSM::layer_up() {
    switch( state ) {
    case PPP_FSM_STATE::Initial:
	    state = PPP_FSM_STATE::Closed;
	    break;

    case PPP_FSM_STATE::Starting:
	    if( auto const &[ action, err ] = send_conf_req(); !err.empty() ) {
            log( "Cannot set layer up: " + err );
        } else {
	        state = PPP_FSM_STATE::Req_Sent;
        }
	    break;

    default:
        break;
    }
}

void PPP_FSM::layer_down() {
    return;
}

void PPP_FSM::open() {
    std::chrono::milliseconds timespan( 1000 ); 
    std::this_thread::sleep_for( timespan );

    switch( state ) {
    case PPP_FSM_STATE::Initial:
        state = PPP_FSM_STATE::Starting;
        //starting()
        break;
    case PPP_FSM_STATE::Closed:
        if( auto const &[ action, err ] = send_conf_req(); !err.empty() ) {
            log( "Cannot set layer up: " + err );
        } else {
	        state = PPP_FSM_STATE::Req_Sent;
        }
        break;
    case PPP_FSM_STATE::Closing:
        state = PPP_FSM_STATE::Stopping;
    case PPP_FSM_STATE::Stopped:
    case PPP_FSM_STATE::Opened:
        // If restart
        break;
    default:
        break;
    }
}

FSM_RET PPP_FSM::recv_conf_ack( std::vector<uint8_t> &inPkt ) {
    log( "recv_conf_ack current state: " + std::to_string( state ) );

    // Parse in case of moved data
    PPPOESESSION_HDR *pppoe = reinterpret_cast<PPPOESESSION_HDR*>( inPkt.data() );
    PPP_LCP *lcp = reinterpret_cast<PPP_LCP*>( pppoe->getPayload() );

    if( lcp->identifier != pkt_id ) {
        return { PPP_FSM_ACTION::NONE, "Packet identifier is not match with our" };
    }

    seen_ack = true;

    switch( state ) {
    case PPP_FSM_STATE::Closed:
    case PPP_FSM_STATE::Stopped:
        // send TERM ACK
        break;
    case PPP_FSM_STATE::Req_Sent:
        state = PPP_FSM_STATE::Ack_Rcvd;
        break;
    case PPP_FSM_STATE::Ack_Rcvd:
        log( "extra ack, but not considering it is like a problem" );
        break;
    case PPP_FSM_STATE::Ack_Sent:
        state = PPP_FSM_STATE::Opened;
        layer_up();
        return { PPP_FSM_ACTION::LAYER_UP, "" };
        break;
    case PPP_FSM_STATE::Opened:
        // Restarting the connection
        send_conf_req();
        state = PPP_FSM_STATE::Req_Sent;
        return { PPP_FSM_ACTION::LAYER_DOWN, "" };
        break;
    default:
        log( "Incorrect state?" );
        break;
    }

    return { PPP_FSM_ACTION::NONE, "" };
}

FSM_RET PPP_FSM::recv_term_req( std::vector<uint8_t> &inPkt ) {
    switch( state ) {
    case PPP_FSM_STATE::Ack_Rcvd:
    case PPP_FSM_STATE::Ack_Sent:
	    state = PPP_FSM_STATE::Req_Sent;
	    break;
    case PPP_FSM_STATE::Opened:
        state = PPP_FSM_STATE::Stopping;
        send_term_ack( inPkt );
        return { PPP_FSM_ACTION::LAYER_DOWN, "" };
	    break;
    default:
       break;
    }

    send_term_ack( inPkt );
    return { PPP_FSM_ACTION::NONE, "" };
}