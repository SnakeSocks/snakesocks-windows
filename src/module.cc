#include <cstring>
#include "module.hpp"
#include "syserr.hpp"
#include "NetLib.hpp"

void client_module::initCString()
{
//    NetLib::ipv4ToBytes(serverIp, conv_serverIp);
    auto servAddr = BoostNetLib::resolveDomainOnce(serverIp);
    if(servAddr.is_v4()) {
        auto _conv_serverIp = servAddr.to_v4().to_bytes();
        memcpy(conv_serverIp, _conv_serverIp.data(), 4);
    }
    if(servAddr.is_v6()) {
        auto _conv_serverIp = servAddr.to_v6().to_bytes();
        memcpy(conv_serverIp, _conv_serverIp.data(), 16);
    }

    conv_passphrase.null_terminated = true;
    conv_passphrase.length = static_cast<uint32_t>(passphrase.size());
    conv_passphrase.str = (char *)calloc(1, passphrase.size() + 1);
    memcpy(conv_passphrase.str, passphrase.c_str(), passphrase.size());
}

void client_module::destroyCString()
{
    free(conv_passphrase.str);
}

binary_safe_string client_module::makeAuthQuery(int connfd)
{
    connect_info ci;
    ci.connect_fd = connfd;
    memcpy(ci.server_ip, conv_serverIp, 16);
    ci.server_port = serverPort;
    ci.passphrase = conv_passphrase;
    return _f_client_make_auth_query(&ci);
}

bool client_module::onAuthReply(char *authdata, size_t len, int connfd)
{
    connect_info ci;
    ci.connect_fd = connfd;
    memcpy(ci.server_ip, conv_serverIp, 16);
    ci.server_port = serverPort;
    ci.passphrase = conv_passphrase;

    binary_safe_string server_reply;
    server_reply.null_terminated = false;
    server_reply.length = static_cast<uint32_t>(len);
    server_reply.str = (char *)authdata;

    return _f_client_deal_auth_reply(&ci, server_reply);
}

binary_safe_string client_module::encode(char *data, size_t len, int connfd, client_query addrTemplate) // MUST copy construct.
{
    connect_info ci;
    ci.connect_fd = connfd;
    memcpy(ci.server_ip, conv_serverIp, 16);
    ci.server_port = serverPort;
    ci.passphrase = conv_passphrase;

    if(len)
    {
        binary_safe_string server_reply;
        server_reply.null_terminated = false;
        server_reply.length = static_cast<uint32_t>(len);
        server_reply.str = (char *)malloc(len);

        memcpy(server_reply.str, data, len);
        addrTemplate.payload = server_reply;
    }

    auto toReturn = _f_client_encode(&ci, addrTemplate);
    //free(server_reply.str);
    return std::move(toReturn);
}

client_query client_module::decode(char *data, size_t len, int connfd)
{
    connect_info ci;
    ci.connect_fd = connfd;
    memcpy(ci.server_ip, conv_serverIp, 16);
    ci.server_port = serverPort;
    ci.passphrase = conv_passphrase;

    binary_safe_string server_reply;
    server_reply.null_terminated = false;
    server_reply.length = static_cast<uint32_t>(len);
    server_reply.str = (char *)malloc(len);

    memcpy(server_reply.str, data, len);

    auto toReturn = _f_client_decode(&ci, server_reply);
    //free(server_reply.str);
    return std::move(toReturn);
}

#include "module-port.hpp"
#ifdef ADAPT_WIN32_DLIB
#include <windows.h>
void client_module::loadSo(const string &filePath)
{
    using namespace ____trick;
    HMODULE handle = LoadLibrary(filePath.c_str());
    if(!handle) sysdie("Failed to open dll dylib %s, LastError=%d. Confirm if it exists and you have access to it.", filePath.c_str(), GetLastError());

    auto _pf_client_make_auth_query = (_pf_client_make_auth_query_t)GetProcAddress(handle, "client_make_auth_query");
    if (_pf_client_make_auth_query == NULL) sysdie("find symbol client_make_auth_query from %s failed. LastError=%d", filePath.c_str(), GetLastError());
    _f_client_make_auth_query = _pf_client_make_auth_query;

    auto _pf_client_deal_auth_reply = (_pf_client_deal_auth_reply_t)GetProcAddress(handle, "client_deal_auth_reply");
    if (_pf_client_deal_auth_reply == NULL) sysdie("find symbol client_deal_auth_reply from %s failed. LastError=%d", filePath.c_str(), GetLastError());
    _f_client_deal_auth_reply = _pf_client_deal_auth_reply;

    auto _pf_client_encode = (_pf_client_encode_t)GetProcAddress(handle, "client_encode");
    if (_pf_client_encode == NULL) sysdie("find symbol client_encode from %s failed. LastError=%d", filePath.c_str(), GetLastError());
    _f_client_encode = _pf_client_encode;

    auto _pf_client_decode = (_pf_client_decode_t)GetProcAddress(handle, "client_decode");
    if (_pf_client_decode == NULL) sysdie("find symbol client_decode from %s failed. LastError=%d", filePath.c_str(), GetLastError());
    _f_client_decode = _pf_client_decode;
    
    so_handle = handle; //Here is master thread. sysdie will exit program and windows will free this handle on error automatically.
                     //No need for scope guard.
    return;
}
#else
#include <dlfcn.h>
void client_module::loadSo(const string &filePath)
{
    using namespace ____trick;
	debug(1) printf("dlopen('%s')", filePath.c_str());
    void *handle = dlopen(filePath.c_str(), RTLD_LAZY);
    char *errstr = NULL;
    if(!handle) sysdie("dlopen failed to open shared object %s, dlerror=%s", filePath.c_str(), dlerror());
    dlerror();

    auto _pf_client_make_auth_query = (_pf_client_make_auth_query_t)dlsym(handle, "client_make_auth_query");
    if ((errstr = dlerror()) != NULL) sysdie("find symbol client_make_auth_query from %s failed. dlerror=%s", filePath.c_str(), dlerror());
    _f_client_make_auth_query = _pf_client_make_auth_query;

    auto _pf_client_deal_auth_reply = (_pf_client_deal_auth_reply_t)dlsym(handle, "client_deal_auth_reply");
    if ((errstr = dlerror()) != NULL) sysdie("find symbol client_deal_auth_reply from %s failed. dlerror=%s", filePath.c_str(), dlerror());
    _f_client_deal_auth_reply = _pf_client_deal_auth_reply;

    auto _pf_client_encode = (_pf_client_encode_t)dlsym(handle, "client_encode");
    if ((errstr = dlerror()) != NULL) sysdie("find symbol client_encode from %s failed. dlerror=%s", filePath.c_str(), dlerror());
    _f_client_encode = _pf_client_encode;

    auto _pf_client_decode = (_pf_client_decode_t)dlsym(handle, "client_decode");
    if ((errstr = dlerror()) != NULL) sysdie("find symbol client_decode from %s failed. dlerror=%s", filePath.c_str(), dlerror());
    _f_client_decode = _pf_client_decode;
    
    so_handle = handle; //Here is master thread. sysdie will exit program and linux will close this handle on error automatically.
                     //No need for scope guard.
    return;
}
#endif
