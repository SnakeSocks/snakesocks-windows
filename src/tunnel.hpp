#ifndef SNAKESOCKS_TUNNEL_HPP
#define SNAKESOCKS_TUNNEL_HPP

#include "module.hpp"
#include "debug.hpp"

#include <string>
#include <boost/core/noncopyable.hpp>
using std::string;
using fd=int;

class tunnel : private boost::noncopyable
{
public:
    tunnel() = delete;
    tunnel(client_module &&cmod) : mod(std::move(cmod)) {RECORD}
    tunnel(tunnel &&ano): mod(std::move(ano.mod)) {}
    fd newConnection(); //Return a new connfd after authorization. If auth failed, return -1;
public:
    bool doAuth(fd connfd);
    client_module mod;
};


#endif //SNAKESOCKS_TUNNEL_HPP
