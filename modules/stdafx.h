#ifndef _SNAKESOCKS_STDAFX_H
#define _SNAKESOCKS_STDAFX_H 1
// All SnakeSocks module must include this header, and implement every function listed below.
// You can use any language freely in your source. But C-style function must be exposed directly
// in compiled object, so that SnakeSocks can import and find needed function by symbol.

// NOTICE: DO NOT FORGET TO CHECK IF STRING_LEN == 0 IF NECESSARY.

#ifdef __cplusplus
extern "C" {
#endif


// Common definition
#include <stdbool.h>
#include <stdint.h>
typedef struct _binary_safe_string
{
    bool null_terminated; //If true, it's OK to either regard str as a null-terminated string, or as a binary_safe_string.
    uint32_t length; //length(empty_string) must be 0.
    char *str; //This string must be created by malloc/calloc, released by free. Last user must free it.
} binary_safe_string;
typedef struct _connect_info
{
    uint8_t server_ip[16]; // notice both ip and port is encoded with network octet order.
    uint16_t server_port; // client_query also follows this regulation.
    binary_safe_string passphrase; //Set from config file.
    int connect_fd; // Activating connection who call you. WARNING : DO NOT execute any operation on the fd given here.
                    // There is no guarantee that any operation applied at this stuff will work or not.
                    // Future version might remove this entry. Suggestions:DO NOT CODE ANYTHING USING THIS STUFF.

} connect_info;
typedef struct _client_query {
    uint8_t destination_ip[16];
    uint16_t destination_port;
    binary_safe_string payload;// data pack to be sent to dest
}client_query;
/* Client-side function declaration --
 *
 * Agreement about malloc and free to binary_safe_string::str:
 *
 * You must NEVER `free` str inside connect_info passed to you.
 * You must NEVER try to edit binary_safe_string inside connect_info. Regard them as const char * please.
 *
 * For `client_deal_auth_reply`:
 * You must NEVER free server_reply.
 *
 * For `client_encode` and `client_decode`:
 * You can either malloc/calloc a new binary_safe_string to return and free `data`, or edit `data` and return it.
 * I'll free returned string only once, and never free `data` passed to client_(en/de)code.
 */
#ifdef COMPILE_SS_MAIN_CLIENT
[[deprecated]]
#endif
binary_safe_string client_make_auth_query(const connect_info *);
#ifdef COMPILE_SS_MAIN_CLIENT
[[deprecated]]
#endif
bool client_deal_auth_reply(const connect_info *, binary_safe_string server_reply); //Return true if auth is ok.
#ifdef COMPILE_SS_MAIN_CLIENT
[[deprecated]]
#endif
binary_safe_string client_encode(const connect_info *,client_query payload); //I'll call it per complete packet.
#ifdef COMPILE_SS_MAIN_CLIENT
[[deprecated]]
#endif
client_query client_decode(const connect_info *, binary_safe_string data); //I'll call it per complete packet.

// Server-side function declaration
// The agreement is mainly the same as client-side function.
// For `server_make_auth_reply`:
// You must free the binary_safe_string pass to you, I'll never free it.
binary_safe_string server_make_auth_reply(connect_info *, binary_safe_string, bool * is_closing);
binary_safe_string server_encode(const connect_info *, client_query payload);
client_query server_decode(const connect_info *,binary_safe_string data);



#ifdef __cplusplus
};
#endif

#endif //_SNAKESOCKS_STDAFX_H