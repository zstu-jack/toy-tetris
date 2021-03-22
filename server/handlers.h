//
// Created by jackyan on 2021/3/22.
//

#ifndef HANDLERS_H
#define HANDLERS_H

class Head;
class TcpConntcion;

void onReqLogin(const TcpConnection* conn, Head& head, const char* msg, int len);
void onGamePlayerLeave(const TcpConnection* conn, Head& head, const char* msg, int len);
void onGamePlayerOp(const TcpConnection* conn, Head& head, const char* msg, int len);
void onReqPlayerMatch(const TcpConnection* conn, Head& head, const char* msg, int len);
void onReqPlayerCancelMatch(const TcpConnection* conn, Head& head, const char* msg, int len);

#endif //HANDLERS_H
