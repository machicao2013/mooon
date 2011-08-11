#ifndef MOOON_ECHO_SERVER_PACKET_HANDLER_IMPL_H
#define MOOON_ECHO_SERVER_PACKET_HANDLER_IMPL_H
#include <server/server.h>

class CPakcetHandlerImpl: public server::IPacketHandler
{
public:    
    CPakcetHandlerImpl(server::IConnection* connection);
    ~CPakcetHandlerImpl();

private:  
    virtual void reset();

    virtual char* get_request_buffer();
    virtual size_t get_request_size() const;        
    virtual util::handle_result_t on_handle_request(size_t data_size, server::Indicator& indicator);
             
    virtual const char* get_response_buffer() const;
    virtual size_t get_response_size() const;
    virtual size_t get_response_offset() const;
    virtual void move_response_offset(size_t offset);
    virtual util::handle_result_t on_response_completed(server::Indicator& indicator);

private:
    server::IConnection* _connection; // 建立的连接
    size_t _request_size;    // 接收请求缓冲区的最大字节数
    size_t _response_size;   // 需要发送的响应数据字节数
    size_t _response_offset; // 当前已经发送的响应数据字节数
    char* _request_buffer;   // 用来存放请求数据的缓冲区
};

#endif // MOOON_ECHO_SERVER_PACKET_HANDLER_IMPL_H
