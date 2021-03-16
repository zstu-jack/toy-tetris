protoc --proto_path=./protocal/ --cpp_out=./protocal/ ./protocal/*.proto

g++ -g3 -std=c++11 server/*.cpp ./protocal/*.cc ./common/*.cpp ./*.cpp ../easy-muduo/common/*.cpp -o tserver -lprotobuf -lncurses
g++ -g3 -std=c++11 client/*.cpp ./protocal/*.cc ./common/*.cpp ./*.cpp ../easy-muduo/common/*.cpp -o tclient -lprotobuf -lncurses