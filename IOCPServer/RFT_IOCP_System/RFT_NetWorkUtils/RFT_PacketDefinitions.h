#pragma once
#include <cstdint>
#include <string>
#include "Protocol/Protocol.pb.h"
// 1. 패킷 송장 (네트워크 헤더)
// #pragma pack(push, 1)은 구조체 사이의 빈 공간(패딩)을 없애고 딱 4바이트로 압축해 줍니다. (네트워크 통신 필수)
#pragma pack(push, 1)
struct PacketHeader 
{
    uint16_t packetSize; // 헤더 크기(4) + Protobuf 데이터 크기
    uint16_t messageId;  // Protocol::MessageId (예: REQ_MATCH)
};
#pragma pack(pop)

constexpr size_t HEADER_SIZE = sizeof(PacketHeader); // 항상 4바이트

// 2. 디버깅 유틸리티 (유지)
void PrintBufferHex(const char* buffer, size_t length);

// 3. 패킷 자동 포장 기계
class PacketHandler 
{
public:
    // Protobuf 구조체를 던져주면 [헤더+데이터]가 합쳐진 완벽한 바이트 배열(string)을 만들어 줍니다.
    static std::string MakeSendBuffer(Protocol::MessageId msgId, const google::protobuf::Message& message);
};