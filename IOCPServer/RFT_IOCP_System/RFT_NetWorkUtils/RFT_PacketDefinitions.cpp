#include "RFT_PacketDefinitions.h"
#include <cstdio>

void PrintBufferHex(const char* buffer, size_t length) 
{
    printf("Buffer (Hex): ");
    for (size_t i = 0; i < length; ++i) 
    {
        printf("%02X ", static_cast<unsigned char>(buffer[i]));
    }
    printf("\n");
}

std::string PacketHandler::MakeSendBuffer(Protocol::MessageId msgId, const google::protobuf::Message& message) 
{
    // 1. 내용물(Protobuf)을 바이트로 압축
    std::string payload = message.SerializeAsString();

    // 2. 송장(헤더) 작성
    PacketHeader header;
    header.packetSize = static_cast<uint16_t>(HEADER_SIZE + payload.size());
    header.messageId = static_cast<uint16_t>(msgId);

    // 3. 박스(sendBuffer)에 송장과 내용물을 차례대로 넣음
    std::string sendBuffer;
    sendBuffer.append(reinterpret_cast<char*>(&header), HEADER_SIZE);
    sendBuffer.append(payload);

    // 4. 완성된 박스 반환 (이걸 그대로 WSASend에 넣으면 끝!)
    return sendBuffer;
}