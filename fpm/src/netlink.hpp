struct Netlink {
private:
    VPPAPI vpp;
public:
    static int data_cb_route( const struct nlmsghdr *nlh, void *data );
    static int data_cb( const struct nlmsghdr *nlh, void *data );
    void process( std::vector<uint8_t> &v);
};