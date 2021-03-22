//
// Created by jackyan on 2021/3/15.
//
#include <time.h>
#include <google/protobuf/message.h>
#include <string>
#include <random>
#include <arpa/inet.h>

#ifndef UTIL_H
#define UTIL_H

class TcpConnection;

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG(LEVEL, X, ...) log(LEVEL, "[%20s:%-3d-%20s] [" X "]\n", __FILENAME__,  __LINE__,__FUNCTION__, ##__VA_ARGS__)

#define PROTO_PARSE(proto_req) \
        if(!proto_req.ParseFromArray(msg,len)) \
        { \
            logger.LOG(ERROR,"parse proto(%s) error body=%p len=%d\n", proto_req.GetTypeName().c_str(),msg,len); \
            return ; \
        }

template<typename T>
T get_random(T l, T r){
    std::mt19937 rnd(time(NULL));
    return rnd() % (r - l + 1) + l;
}

std::string packMessage(int msgid, int uin, google::protobuf::Message& message);
std::string packMessage(int msgid, int uin, int msg_len = 12);

// == -1  error, which would lead to close the connection
// >  0   package size.
// == 0   wait for head.
int decodeMessage(const TcpConnection* conn, const char* msg, int len);
long long get_1970_ms();



// other useful functions
// return -1 if element not exist.
// return index of first element satisfy the condition.
template<typename Iter, typename T>
int index_of(Iter begin, Iter end, T element){
    int index = 0;
    for(auto iter = begin; iter != end; iter ++){
        if (*iter == element){
            return index;
        }
        index ++;
    }
    return -1;
}

#endif //UTIL_H
