#include <sys/time.h>

#include "util.h"
#include "common_define.h"

std::string packMessage(int msgid, int uin, google::protobuf::Message& message){
    std::string body = message.SerializeAsString();
    int msg_len = body.size() + 12;
    return packMessage(msgid, uin, msg_len) + body;
}
std::string packMessage(int msgid, int uin, int msg_len){
    static char buff[SOCKET_APP_MAX_BUFFER_SIZE];
    *(uint32_t*)buff = htonl(msg_len);
    *(uint32_t*)(buff+4) = htonl(msgid);
    *(uint32_t*)(buff+8) = htonl(uin);
    return std::string(buff, 12);
}
int decodeMessage(const TcpConnection* conn, const char* msg, int len){
    if(len < 4) return 0;
    auto pkgSize = ntohl(*(int32_t *) msg);
    if(pkgSize >= SOCKET_APP_MAX_BUFFER_SIZE || pkgSize < 0){
        return -1;
    }
    return pkgSize;
}

long long get_1970_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}