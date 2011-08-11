#include <net/util.h>
#include "config_impl.h"

bool CConfigImpl::init(uint16_t port)
{
    try
    {        
        net::eth_ip_array_t eth_ip_array;
        net::CUtil::get_ethx_ip(eth_ip_array);

        // 设置默认的监听端口
        if (0 == port) 
        {
            port = 2012;
        }

        // 取得网卡上所有的IP地址
        for (int i=0; i<(int)eth_ip_array.size(); ++i)
        {
            net::ip_port_pair_t ip_port_pair;
            ip_port_pair.first = eth_ip_array[i].second.c_str();
            ip_port_pair.second = port;

            _ip_port_pair_array.push_back(ip_port_pair);            
        }
        
        return !_ip_port_pair_array.empty();
    }
    catch (sys::CSyscallException& ex)
    {
        fprintf(stderr, "Get IP error: %s.\n", ex.to_string().c_str());
        return false;
    }
}

const net::ip_port_pair_array_t& CConfigImpl::get_listen_parameter() const
{
    return _ip_port_pair_array;
}
