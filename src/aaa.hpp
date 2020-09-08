#ifndef AAA_HPP_
#define AAA_HPP_

#include "auth_client.hpp"
#include "session.hpp"

struct AAAConf;
struct PPPOELocalTemplate;
struct RadiusResponse;

#define SESSION_ERROR UINT32_MAX

using aaa_callback = std::function<void(uint32_t,std::string)>;

class AuthClient;

class AAA_Session : public std::enable_shared_from_this<AAA_Session> {
public:
    AAA_Session() = default;
    AAA_Session( const AAA_Session & ) = delete;
    AAA_Session( AAA_Session && ) = default;
    AAA_Session& operator=( const AAA_Session& ) = delete;
    AAA_Session& operator=( AAA_Session&& ) = default;

    AAA_Session( io_service &i, uint32_t sid, const std::string &u, PPPOELocalTemplate &t );
    AAA_Session( io_service &i, uint32_t sid, const std::string &u, PPPOELocalTemplate &t, RadiusResponse resp, std::shared_ptr<AuthClient> s );
    ~AAA_Session();

    io_service &io;
    boost::asio::steady_timer timer;
    uint32_t session_id;
    uint32_t ifindex;
    std::string username;
    address_v4_t address;

    address_v4_t dns1;
    address_v4_t dns2;

    std::shared_ptr<AuthClient> acct { nullptr };
    PPPOELocalTemplate &templ;
    bool free_ip { false };
    bool to_stop_acct{ false };

    void start();
    void stop();
    void on_started( RADIUS_CODE code, std::vector<uint8_t> pkt );
    void on_interim_answer( RADIUS_CODE code, std::vector<uint8_t> pkt );
    void on_stopped( RADIUS_CODE code, std::vector<uint8_t> pkt );
    void on_failed( std::string err );
    void on_interim( const boost::system::error_code& error );
};

class AAA {
    io_service &io;
    AAAConf &conf;
    std::map<uint32_t,std::shared_ptr<AAA_Session>> sessions;
    std::map<std::string,std::shared_ptr<AuthClient>> auth;
    std::map<std::string,std::shared_ptr<AuthClient>> acct;


    // radius methods
    void startSessionRadius( const std::string &user, const std::string &pass, PPPOESession &sess, aaa_callback callback );
    void startSessionRadiusChap( const std::string &user, const std::string &challenge, const std::string &response, PPPOESession &sess, aaa_callback callback );
    void processRadiusAnswer( aaa_callback callback, std::string user, RADIUS_CODE code, std::vector<uint8_t> v );
    void processRadiusError( aaa_callback callback, const std::string &error );
    // local and none methods
    std::tuple<uint32_t,std::string> startSessionNone( const std::string &user, const std::string &pass );

public:
    AAA( io_service &i, AAAConf &c );

    std::tuple<std::shared_ptr<AAA_Session>,std::string> getSession( uint32_t sid );
    void startSession( const std::string &user, const std::string &pass, PPPOESession &sess, aaa_callback callback );
    void startSessionCHAP( const std::string &user, const std::string &challenge, const std::string &response, PPPOESession &sess, aaa_callback callback );
    void stopSession( uint32_t sid );
    void mapIfaceToSession( uint32_t session_id, uint32_t ifindex );

    std::optional<RadiusDict> dict;
};

#endif