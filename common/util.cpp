#include "util.h"

extern const int MAX_BUFFER_SIZE = 10240;

std::string packMessage(int msgid, int uin, google::protobuf::Message& message){
    static char buff[MAX_BUFFER_SIZE];
    std::string body = message.SerializeAsString();
    int msg_len = body.size() + 12;
    assert(msg_len <= MAX_BUFFER_SIZE);

    *(uint32_t*)buff = htonl(msg_len);
    *(uint32_t*)(buff+4) = htonl(msgid);
    *(uint32_t*)(buff+8) = htonl(uin);
    return std::string(buff, 12) + body;
}