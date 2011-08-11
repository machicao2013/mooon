#ifndef MOOON_ECHO_SERVER_FACTORY_IMPL_H
#define MOOON_ECHO_SERVER_FACTORY_IMPL_H
#include <server/server.h>

class CFactoryImpl: public server::IFactory
{
private:
    virtual server::IPacketHandler* create_packet_handler(server::IConnection* connection);
};

#endif // MOOON_ECHO_SERVER_FACTORY_IMPL_H
