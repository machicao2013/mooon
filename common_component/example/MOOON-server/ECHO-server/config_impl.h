#ifndef MOOON_ECHO_SERVER_CONFIG_IMPL_H
#define MOOON_ECHO_SERVER_CONFIG_IMPL_H
#include <server/server.h>

class CConfigImpl: public server::IConfig
{
public:
    bool init(uint16_t port);

private:
    virtual const net::ip_port_pair_array_t& get_listen_parameter() const;

private:
    net::ip_port_pair_array_t _ip_port_pair_array;
};

#endif // MOOON_ECHO_SERVER_CONFIG_IMPL_H
