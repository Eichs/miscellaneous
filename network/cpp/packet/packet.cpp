#include "packet.h"
#include <iostream>
#include <cstring>


std::vector<TcpMsg> DecodeBinToPayload(const std::vector<uint8_t>& data, uint32_t sessionId) {
    std::vector<TcpMsg> tcpMsgList;
    size_t offset = 0;

    while (offset + 12 <= data.size()) {
        if (data[offset] != 0x45 || data[offset + 1] != 0x67) {
            std::cerr << "packet head magic 0x4567 error\n";
            break;
        }

        uint16_t cmdId = (data[offset + 2] << 8) | data[offset + 3];
        int headLen = (data[offset + 4] << 8) | data[offset + 5];
        int protoLen = (data[offset + 6] << 24) | (data[offset + 7] << 16) | (data[offset + 8] << 8) | data[offset + 9];
        int packetLen = headLen + protoLen + 12;

        if (packetLen > PacketMaxLen || offset + packetLen > data.size()) {
            std::cerr << "packet length error\n";
            break;
        }

        if (data[offset + packetLen - 2] != 0x89 || data[offset + packetLen - 1] != 0xAB) {
            std::cerr << "packet tail magic 0x89AB error\n";
            break;
        }

        TcpMsg tcpMsg;
        tcpMsg.SessionId = sessionId;
        tcpMsg.CmdId = cmdId;
        tcpMsg.HeadData.assign(data.begin() + offset + 10, data.begin() + offset + 10 + headLen);
        tcpMsg.ProtoData.assign(data.begin() + offset + 10 + headLen, data.begin() + offset + 10 + headLen + protoLen);

        tcpMsgList.push_back(tcpMsg);
        offset += packetLen;
    }

    return tcpMsgList;
}

std::vector<uint8_t> EncodePayloadToBin(const TcpMsg& tcpMsg) {    int headLen = tcpMsg.HeadData.size();
    int protoLen = tcpMsg.ProtoData.size();
    int packetLen = headLen + protoLen + 12;
    std::vector<uint8_t> bin(packetLen);

    bin[0] = 0x45;
    bin[1] = 0x67;
    bin[2] = (tcpMsg.CmdId >> 8) & 0xFF;
    bin[3] = tcpMsg.CmdId & 0xFF;
    bin[4] = (headLen >> 8) & 0xFF;
    bin[5] = headLen & 0xFF;
    bin[6] = (protoLen >> 24) & 0xFF;
    bin[7] = (protoLen >> 16) & 0xFF;
    bin[8] = (protoLen >> 8) & 0xFF;
    bin[9] = protoLen & 0xFF;
    std::memcpy(bin.data() + 10, tcpMsg.HeadData.data(), headLen);
    std::memcpy(bin.data() + 10 + headLen, tcpMsg.ProtoData.data(), protoLen);
    bin[packetLen - 2] = 0x89;
    bin[packetLen - 1] = 0xAB;

    return bin;
}

