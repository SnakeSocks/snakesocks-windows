#ifndef _SNAKESOCKS_PORT_HPP
#define _SNAKESOCKS_PORT_HPP 1

#include <string>
#include <functional>
#include <boost/core/noncopyable.hpp>
#include "../modules/stdafx.h"
#include "debug.hpp"
#include "module-port.hpp"

#ifdef ADAPT_WIN32_DLIB
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using std::string;

class client_module : private boost::noncopyable
{
public:
    client_module() = delete;
    client_module(const string &ip, uint16_t port, const string &psFromCfg, const string &soPath)
            : serverIp(ip), serverPort(port), passphrase(psFromCfg) {initCString(); loadSo(soPath);}
    client_module(client_module &&ano)
            : serverIp(std::move(ano.serverIp)), passphrase(std::move(ano.passphrase)), serverPort(ano.serverPort),
            _f_client_make_auth_query(std::move(ano._f_client_make_auth_query)),
            _f_client_deal_auth_reply(std::move(ano._f_client_deal_auth_reply)),
            _f_client_encode(std::move(ano._f_client_encode)),
            _f_client_decode(std::move(ano._f_client_decode)),
            so_handle(ano.so_handle) {initCString(); ano.so_handle = nullptr; RECORD}
    ~client_module() {
        destroyCString();
        if(so_handle)
#ifdef ADAPT_WIN32_DLIB
			FreeLibrary(so_handle);
#else
			dlclose(so_handle);
#endif
    }
    void initCString();
    void destroyCString();

    void loadSo(const string &filePath);

    //Functions below must be thread safe.
    binary_safe_string makeAuthQuery(int connfd);
    bool onAuthReply(char *authdata, size_t len, int connfd);
    binary_safe_string encode(char *data, size_t len, int connfd, client_query payload_template);
    client_query decode(char *data, size_t len, int connfd);
public:
    string serverIp;
    uint16_t serverPort;
    string passphrase;
private:
    std::function<binary_safe_string (const connect_info *)> _f_client_make_auth_query;
    std::function<_Bool (const connect_info *, binary_safe_string)> _f_client_deal_auth_reply;
    std::function<binary_safe_string (const connect_info *, client_query)> _f_client_encode;
    std::function<client_query (const connect_info *, binary_safe_string)> _f_client_decode;
#ifdef ADAPT_WIN32_DLIB
	HMODULE so_handle;
#else
    void *so_handle;
#endif

    uint8_t conv_serverIp[16] = { 0 };
    binary_safe_string conv_passphrase;
};

#endif //_SNAKESOCKS_PORT_HPPred
