#include "MainLobbyUserWidget.h"
#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Kismet/GameplayStatics.h"
#include "RFTGameInstance.h"
#include "Async/Async.h"
#include "Protocol/ProtobufWrapper.h"

#pragma pack(push, 1)
struct FPacketHeader
{
    uint16 PacketSize;
    uint16 MessageId;
};
#pragma pack(pop)

// Runnable 클래스 정의
class FMatchNetworkRunnable : public FRunnable
{
public:
    FSocket* MatchSocket;
    FThreadSafeBool bIsRunning;
    TFunction<void(const FString&, uint8)> OnResponseReceived;

    FMatchNetworkRunnable(FSocket* InSocket, TFunction<void(const FString&, uint8)> ResponseCallback)
        : MatchSocket(InSocket), bIsRunning(true), OnResponseReceived(ResponseCallback)
    {}

    virtual uint32 Run() override
    {
        UE_LOG(LogTemp, Warning, TEXT("[NetworkThread] Run Start"));

        while (bIsRunning)
        {
            if (MatchSocket && MatchSocket->GetConnectionState() == SCS_Connected)
            {
                // 1. 소켓에 읽을 데이터가 올 때까지 스레드를 대기시킵니다. (CPU 낭비 방지!)
                uint32 PendingDataSize = 0;
                if (MatchSocket->HasPendingData(PendingDataSize))
                {
                    // 2. 먼저 '헤더(4바이트)'만큼만 데이터를 읽어옵니다.
                    FPacketHeader Header;
                    int32 BytesRead = 0;

                    // Peek(훔쳐보기) 기능이 없으므로 일단 4바이트를 강제로 읽어옵니다.
                    if (MatchSocket->Recv((uint8*)&Header, sizeof(FPacketHeader), BytesRead) && BytesRead == sizeof(FPacketHeader))
                    {
                        int32 PayloadSize = Header.PacketSize - sizeof(FPacketHeader);

                        // 3. 헤더에 적힌 내용물 크기(PayloadSize)만큼 나머지 데이터를 읽어옵니다.
                        TArray<uint8> PayloadBuffer;
                        PayloadBuffer.SetNumUninitialized(PayloadSize);
                        int32 PayloadBytesRead = 0;

                        if (MatchSocket->Recv(PayloadBuffer.GetData(), PayloadSize, PayloadBytesRead) && PayloadBytesRead == PayloadSize)
                        {
                            // 4. 순수 Protobuf 데이터를 FString으로 변환하여 메인 스레드로 넘김
                            // (Protobuf는 중간에 널문자가 있을 수 있으므로 길이를 명시해서 FString 생성)
                            FString Data(PayloadSize, UTF8_TO_TCHAR(reinterpret_cast<const char*>(PayloadBuffer.GetData())));

                            if (OnResponseReceived) {
                                OnResponseReceived(Data, Header.MessageId);
                            }
                        }
                    }
                }
                else {
                    // 데이터가 없으면 아주 잠깐(10ms) 쉬면서 무한루프 속도 조절
                    FPlatformProcess::Sleep(0.01f);
                }
            }
            else {
                break; // 연결 끊김
            }
        }
        return 0;
    }

    void Stop()
    {
        bIsRunning = false;
    }

    bool SendMatchRequest(const TArray<uint8>& Packet)
    {
        if (MatchSocket && MatchSocket->GetConnectionState() == SCS_Connected)
        {
            int32 BytesSent;
            return MatchSocket->Send(Packet.GetData(), Packet.Num(), BytesSent);
        }
        return false;
    }
};


// MainLobbyUserWidget 멤버 변수 초기화
FMatchNetworkRunnable* MatchNetworkRunnable = nullptr;
FRunnableThread* MatchNetworkThread = nullptr;

void UMainLobbyUserWidget::NativeConstruct()
{
    if (matching)
    {
        matching->OnClicked.AddDynamic(this, &UMainLobbyUserWidget::OnMatchButtonClicked);
    }

    if (Join)
    {
        Join->OnClicked.AddDynamic(this, &UMainLobbyUserWidget::OnJoinButtonClicked);
    }
}

void UMainLobbyUserWidget::OnJoinButtonClicked()
{
    FString ServerAddress = TEXT("127.0.0.1");

    // OpenLevel 함수는 해당 주소로 연결을 시도합니다. 일반적으로 7777은 기본 포트입니다.
    FString Command = FString::Printf(TEXT("%s:%d"), *ServerAddress, 7777);

    // 127.0.0.1 서버에 연결 (예: 멀티플레이어 서버)
    UGameplayStatics::OpenLevel(this, FName(*Command), true);
}

void UMainLobbyUserWidget::OnMatchButtonClicked()
{
    if (ConnectToServer())
    {
        TArray<uint8> Packet;
        CreateMatchRequestPacket(Packet);

        // MatchNetworkRunnable 초기화 확인
        if (!MatchNetworkRunnable)
        {
            MatchNetworkRunnable = new FMatchNetworkRunnable(MatchSocket,
                [this](const FString& Data, uint8 Header)
                {
                    AsyncTask(ENamedThreads::GameThread, [this, Data, Header]()
                        {
                            HandleServerResponse(Data, Header);
                        });
                });
        }

        // 패킷 전송 확인
        if (MatchNetworkRunnable && MatchNetworkRunnable->SendMatchRequest(Packet))
        {
            UE_LOG(LogTemp, Log, TEXT("Match request sent successfully."));
            StartNetworkThread(); // 네트워크 스레드 시작
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("send failed"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ConnectToServer failed."));
    }
}

bool UMainLobbyUserWidget::ConnectToServer()
{
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    MatchSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("MatchmakingSocket"), false);
    // 비블로킹 소켓 설정
    MatchSocket->SetNonBlocking(true);
    FIPv4Address IP;
    FIPv4Address::Parse(TEXT("127.0.0.1"), IP);

    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    Addr->SetIp(IP.Value);
    Addr->SetPort(9000);

    ConnectStatus = MatchSocket->Connect(*Addr);
    if (ConnectStatus)
    {
        UE_LOG(LogTemp, Warning, TEXT("Connected to matchmaking server"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to connect to matchmaking server"));
    }
    return ConnectStatus;
}

void UMainLobbyUserWidget::CreateMatchRequestPacket(TArray<uint8>& Packet)
{
    // 1. Protobuf 요청 객체 생성 및 세팅
    Protocol::C2S_MatchRequest req;

    // (주의: 서버에서 player_id를 int32로 받기로 했으므로, FString인 CID를 int로 변환해야 합니다.)
    URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (GameInstance) {
        req.set_player_id(FCString::Atoi(*GameInstance->GetCID()));
    }

    // Protobuf를 바이트 문자열로 직렬화
    std::string payload = req.SerializeAsString();

    // 2. 4바이트 송장(헤더) 작성
    FPacketHeader Header;
    Header.PacketSize = sizeof(FPacketHeader) + payload.size();
    Header.MessageId = Protocol::REQ_MATCH; // Enum 값 (1)

    // 3. TArray에 [헤더 + 데이터] 순서대로 밀어 넣기
    Packet.SetNum(Header.PacketSize);
    FMemory::Memcpy(Packet.GetData(), &Header, sizeof(FPacketHeader));
    FMemory::Memcpy(Packet.GetData() + sizeof(FPacketHeader), payload.c_str(), payload.size());

    UE_LOG(LogTemp, Warning, TEXT("[Network] REQ_MATCH 패킷 전송 준비 완료! 크기: %d"), Header.PacketSize);
}

void UMainLobbyUserWidget::HandleServerResponse(const FString& Data, uint8 HeaderId)
{
    switch (HeaderId)
    {
    case Protocol::RES_MATCH_SUCCESS:
    {
        // 1. FString으로 넘어온 바이트 배열을 다시 Protobuf 객체로 변환 (역직렬화)
        Protocol::S2C_MatchSuccess res;
        std::string payload(TCHAR_TO_UTF8(*Data), Data.Len());

        if (res.ParseFromString(payload))
        {
            FString ServerIP = FString(UTF8_TO_TCHAR(res.ds_ip().c_str()));
            int32 ServerPort = res.ds_port();

            UE_LOG(LogTemp, Warning, TEXT("[Network]  매칭 완료! 서버로 이동합니다: %s:%d"), *ServerIP, ServerPort);

            Mcomp = true;
            StopNetworkThread(); // 매칭이 끝났으니 수신 스레드 종료

            // 2. 매칭 서버가 알려준 진짜 데디케이티드 서버 주소로 맵 이동!
            FString Command = FString::Printf(TEXT("%s:%d"), *ServerIP, ServerPort);
            UGameplayStatics::OpenLevel(this, FName(*Command), true);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Network] RES_MATCH_SUCCESS 파싱 실패!"));
        }
        break;
    }
    default:
        UE_LOG(LogTemp, Warning, TEXT("[Network] 알 수 없는 헤더 ID: %d"), HeaderId);
        break;
    }
}

void UMainLobbyUserWidget::StartNetworkThread()
{
    MatchNetworkRunnable = new FMatchNetworkRunnable(MatchSocket,
        [this](const FString& Data, uint8 Header)
        {
            AsyncTask(ENamedThreads::GameThread, [this, Data, Header]()
                {
                    if (Data.IsEmpty())
                    {
                        UE_LOG(LogTemp, Error, TEXT("Received empty data. Skipping HandleServerResponse."));
                        return;
                    }

                    HandleServerResponse(Data, Header);
                });
        });

    MatchNetworkThread = FRunnableThread::Create(MatchNetworkRunnable, TEXT("MatchNetworkThread"));
}

void UMainLobbyUserWidget::StopNetworkThread()
{
    if (MatchNetworkRunnable)
    {
        MatchNetworkRunnable->Stop();
        delete MatchNetworkRunnable;
        MatchNetworkRunnable = nullptr;
    }

    if (MatchNetworkThread)
    {
        MatchNetworkThread->Kill(true);
        delete MatchNetworkThread;
        MatchNetworkThread = nullptr;
    }
}
