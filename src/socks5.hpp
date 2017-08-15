#ifndef SNAKESOCKS_SOCKS5_HPP
#define SNAKESOCKS_SOCKS5_HPP

#include "tunnel.hpp"
#include <winsock2.h>

#include <string>
#include <boost/core/noncopyable.hpp>
using std::string;

class s5server : private boost::noncopyable
{
public:
    s5server() = delete;
    s5server(const string &ip, uint16_t port, tunnel &&sss) : bindIp(ip), bindPort(port), ssserver(std::move(sss)){RECORD}
    [[noreturn]] void run(); //sync
private:
    auto unpackConnectionPacket(const char *pkgStr);
    void handshake(SOCKET connfd);
    client_query doConnect(SOCKET connfd); //return a client_query
    void passPackets(SOCKET connfd, client_query);
    void _dealConnection(SOCKET connfd);
    void dealConnection(SOCKET connfd);
    tunnel ssserver; //I need to get fd_nexthop from ssserver.newConnection
    string bindIp; //raw ipv4. Leave empty for 0.0.0.0(Any)
    uint16_t bindPort;
};


#endif //SNAKESOCKS_SOCKS5_HPP
