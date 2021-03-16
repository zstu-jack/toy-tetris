#include "head.h"

Head::Head(int len,int id,int uid){
    this->msg_len = len;
    this->msg_id = id;
    this->uid = uid;
}
std::string Head::to_string(){
    return std::string("len = ") + std::to_string(this->msg_len) + " id = " + std::to_string(this->msg_id) + " uin = " + std::to_string(this->uid);
}
int Head::size(){
    return sizeof(this->msg_len) + sizeof(this->msg_id) + sizeof(this->uid);
}