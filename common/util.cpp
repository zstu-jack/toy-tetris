#include <sys/time.h>

#include "util.h"
#include "common_define.h"

std::string packMessage(int msgid, int uin, google::protobuf::Message& message){
    static char buff[SOCKET_APP_MAX_BUFFER_SIZE];
    std::string body = message.SerializeAsString();
    int msg_len = body.size() + 12;
    assert(msg_len <= SOCKET_APP_MAX_BUFFER_SIZE);

    *(uint32_t*)buff = htonl(msg_len);
    *(uint32_t*)(buff+4) = htonl(msgid);
    *(uint32_t*)(buff+8) = htonl(uin);
    return std::string(buff, 12) + body;
}
long long get_1970_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}