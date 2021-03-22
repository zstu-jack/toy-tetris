//
// Created by jackyan on 2021/3/22.
//

#ifndef HANDLERS_H
#define HANDLERS_H

void onRspLogin(const TcpConnection* conn, Head& head, const char* msg, int len);
void onRspPlayerMatch(const TcpConnection* conn, Head& head, const char* msg, int len);
void onInfPlayerMatchSuccess(const TcpConnection* conn, Head& head, const char* msg, int len);
void RspGamePlayerLeave(const TcpConnection* conn, Head& head, const char* msg, int len);
void onInfGamePlayerOp(const TcpConnection* conn, Head& head, const char* msg, int len);

#endif //HANDLERS_H
