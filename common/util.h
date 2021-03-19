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

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG(LEVEL, X, ...) log(LEVEL, "[%20s:%-3d-%20s] [" X "]\n", __FILENAME__,  __LINE__,__FUNCTION__, ##__VA_ARGS__)

template<typename T>
T get_random(T l, T r){
    std::mt19937 rnd(time(NULL));
    return rnd() % (r - l + 1) + l;
}

std::string packMessage(int msgid, int uin, google::protobuf::Message& message);

long long get_1970_ms();

#endif //UTIL_H
