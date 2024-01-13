#include <iostream>
#include <vector>
#include <memory>
#include <cstdint>
#include "includes.h"
#include <fstream>
using boost::asio::ip::tcp;


// 声明Session类
class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(tcp::socket socket, uint32_t session_id = 0)
        : socket_(std::move(socket)), session_id_(session_id) {
        initializeCmdidToMessage(cmdidToMessage);
    }

    void start() {
        do_read();
    }

private:
    tcp::socket socket_;
    uint32_t session_id_;
    static constexpr size_t kMaxLength = 1024;
    uint8_t data_[kMaxLength];
    std::map<uint32_t, google::protobuf::Message*> cmdidToMessage;

    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, kMaxLength),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    processReadData(std::vector<uint8_t>(data_, data_ + length));
                    do_read();
                }
            });
    }

    void processReadData(const std::vector<uint8_t>& data) {
        auto tcpMsgList = DecodeBinToPayload(data, session_id_);
        for (const auto& msg : tcpMsgList) {
            handleMessage(msg);
        }
    }

    void handleMessage(const TcpMsg& msg) {
        std::cout << "SessionId: " << msg.SessionId << "\n";
        std::cout << "CmdId: " << msg.CmdId << "\n";

        parseAndPrintHeadData(msg.HeadData);

        auto it = cmdidToMessage.find(msg.CmdId);
        if (it != cmdidToMessage.end()) {
            parseAndPrintProtoData(*it->second, msg.ProtoData);
        } else {
            std::cerr << "Unknown cmdId: " << msg.CmdId << "\n";
        }
    }

    void parseAndPrintHeadData(const std::vector<uint8_t>& head_data) {
        proto::PacketHead head;  // 确保这是一个protobuf消息类型
        if (head.ParseFromArray(head_data.data(), head_data.size())) {
            std::string headDataJson = messageToJsonStr(head);
            std::cout << "HeadData: " << headDataJson << "\n";
        } else {
            std::cerr << "Failed to parse HeadData\n";
        }
    }

    void parseAndPrintProtoData(google::protobuf::Message& message, const std::vector<uint8_t>& proto_data) {
        if (message.ParseFromArray(proto_data.data(), proto_data.size())) {
            std::string protoDataJson = messageToJsonStr(message);
            std::cout << "ProtoData: " << protoDataJson << "\n";
        } else {
            std::cerr << "Failed to parse ProtoData\n";
        }
    }
    std::string messageToJsonStr(const google::protobuf::Message& message) {
        std::string json;
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        options.always_print_primitive_fields = true;
        options.preserve_proto_field_names = true;
                           
        google::protobuf::util::MessageToJsonString(message, &json, options); 
        return json;
    }

};

// TcpServer类管理网络服务
class TcpServer {
public:
    TcpServer(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          socket_(io_context) {
        do_accept();
    }

private:
    tcp::acceptor acceptor_;
    tcp::socket socket_;

    // 接受新的连接
    void do_accept() {
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket_))->start();
            }
            do_accept();
        });
    }
};

int main() {
    try {
        std::ifstream i("config.json");
        nlohmann::json j;
        i >> j;

        std::string address = j["address"];
        short port = j["port"];

        boost::asio::io_context io_context;
        TcpServer server(io_context, port);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
