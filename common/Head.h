#include <string>

class Head{
public:
    int msg_len;
    int msg_id;
    int uid;
    Head(int len,int id,int uid);
    std::string to_string();
    int size();
};