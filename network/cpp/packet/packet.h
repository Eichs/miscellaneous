#ifndef TCPMSG_H
#define TCPMSG_H

#include <vector>
#include <cstdint>


const int PacketMaxLen = 343 * 1024;

struct TcpMsg {
    uint32_t SessionId;
    uint16_t CmdId;
    std::vector<uint8_t> HeadData;
    std::vector<uint8_t> ProtoData;
};


// 声明解码函数
std::vector<TcpMsg> DecodeBinToPayload(const std::vector<uint8_t>& data, uint32_t sessionId);

// 声明编码函数
std::vector<uint8_t> EncodePayloadToBin(const TcpMsg& tcpMsg);

#endif // TCPMSG_H
