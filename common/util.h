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

template<typename T>
T get_random(T l, T r){
    std::mt19937 rnd(time(0));
    return rnd() % (r - l + 1) + l;
}

std::string packMessage(int msgid, int uin, google::protobuf::Message& message);

long long get_1970_ms();

#endif //UTIL_H
