#include "factory_impl.h"
#include "packet_handler_impl.h"

server::IPacketHandler* CFactoryImpl::create_packet_handler(server::IConnection* connection)
{
    return new CPakcetHandlerImpl(connection);
}
