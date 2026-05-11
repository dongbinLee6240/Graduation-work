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

// ── Runnable 클래스 (기존과 동일하지만 구조를 깔끔하게 유지) ──
class FMatchNetworkRunnable : public FRunnable
{
public:
    FSocket* MatchSocket;
    FThreadSafeBool bIsRunning;
    TFunction<void(const FString&, uint8)> OnResponseReceived;

    FMatchNetworkRunnable(FSocket* InSocket, TFunction<void(const FString&, uint8)> ResponseCallback)
        : MatchSocket(InSocket), bIsRunning(true), OnResponseReceived(ResponseCallback)
    {
    }

    virtual uint32 Run() override
    {
        UE_LOG(LogTemp, Warning, TEXT("[NetworkThread] Run Start"));

        while (bIsRunning)
        {
            if (MatchSocket && MatchSocket->GetConnectionState() == SCS_Connected)
            {
                uint32 PendingDataSize = 0;
                if (MatchSocket->HasPendingData(PendingDataSize))
                {
                    FPacketHeader Header;
                    int32 BytesRead = 0;

                    if (MatchSocket->Recv((uint8*)&Header, sizeof(FPacketHeader), BytesRead) && BytesRead == sizeof(FPacketHeader))
                    {
                        int32 PayloadSize = Header.PacketSize - sizeof(FPacketHeader);
                        TArray<uint8> PayloadBuffer;
                        PayloadBuffer.SetNumUninitialized(PayloadSize);
                        int32 PayloadBytesRead = 0;

                        if (MatchSocket->Recv(PayloadBuffer.GetData(), PayloadSize, PayloadBytesRead) && PayloadBytesRead == PayloadSize)
                        {
                            FString Data(PayloadSize, UTF8_TO_TCHAR(reinterpret_cast<const char*>(PayloadBuffer.GetData())));

                            if (OnResponseReceived) {
                                OnResponseReceived(Data, Header.MessageId);
                            }
                        }
                    }
                }
                else {
                    FPlatformProcess::Sleep(0.01f);
                }
            }
            else {
                break;
            }
        }
        return 0;
    }

    void Stop() { bIsRunning = false; }

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

// ── 메인 로비 UI 로직 ──

void UMainLobbyUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

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
    FString Command = FString::Printf(TEXT("%s:%d"), *ServerAddress, 7777);
    UGameplayStatics::OpenLevel(this, FName(*Command), true);
}

void UMainLobbyUserWidget::OnMatchButtonClicked()
{
    if (ConnectToServer())
    {
        TArray<uint8> Packet;
        CreateMatchRequestPacket(Packet);

        // [수정] 스레드를 먼저 깔끔하게 하나만 생성하고 시작합니다. (이중 new 제거)
        StartNetworkThread();

        // [수정] 스레드가 생성된 상태에서 안전하게 패킷을 보냅니다.
        if (MatchNetworkRunnable && MatchNetworkRunnable->SendMatchRequest(Packet))
        {
            // 바이너리 데이터가 깨져서(외계어) 보이지 않도록 크기만 출력합니다.
            UE_LOG(LogTemp, Log, TEXT("Match request sent successfully. Packet Size: %d"), Packet.Num());
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

    // [수정] 논블로킹 해제! 안정적인 Connect와 Send를 위해 기본(블로킹) 모드를 사용합니다.
    // MatchSocket->SetNonBlocking(true); 

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
    Protocol::C2S_MatchRequest req;

    URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (GameInstance) {
        req.set_player_id(FCString::Atoi(*GameInstance->GetCID()));
    }

    std::string payload = req.SerializeAsString();

    FPacketHeader Header;
    Header.PacketSize = sizeof(FPacketHeader) + payload.size();
    Header.MessageId = Protocol::REQ_MATCH;

    Packet.SetNum(Header.PacketSize);
    FMemory::Memcpy(Packet.GetData(), &Header, sizeof(FPacketHeader));

    // [수정] payload.c_str() 대신 바이너리에 안전한 payload.data() 사용
    FMemory::Memcpy(Packet.GetData() + sizeof(FPacketHeader), payload.data(), payload.size());

    UE_LOG(LogTemp, Warning, TEXT("[Network] REQ_MATCH 패킷 전송 준비 완료! 크기: %d"), Header.PacketSize);
}

void UMainLobbyUserWidget::HandleServerResponse(const FString& Data, uint8 HeaderId)
{
    switch (HeaderId)
    {
    case Protocol::RES_MATCH_SUCCESS:
    {
        Protocol::S2C_MatchSuccess res;
        std::string payload(TCHAR_TO_UTF8(*Data), Data.Len());

        if (res.ParseFromString(payload))
        {
            FString ServerIP = FString(UTF8_TO_TCHAR(res.ds_ip().c_str()));
            int32 ServerPort = res.ds_port();

            UE_LOG(LogTemp, Warning, TEXT("[Network] 매칭 완료! 서버로 이동합니다: %s:%d"), *ServerIP, ServerPort);

            // [추가] 맵 이동 전에 소켓 연결을 끊고 스레드를 완전히 종료합니다.
            StopNetworkThread();
            if (MatchSocket)
            {
                MatchSocket->Close();
                MatchSocket = nullptr;
            }

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
    // [추가] 혹시라도 기존 스레드가 돌고 있다면 안전하게 끄고 새로 만듭니다.
    StopNetworkThread();

    MatchNetworkRunnable = new FMatchNetworkRunnable(MatchSocket,
        [this](const FString& Data, uint8 Header)
        {
            AsyncTask(ENamedThreads::GameThread, [this, Data, Header]()
                {
                    if (Data.IsEmpty()) return;
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
    }

    if (MatchNetworkThread)
    {
        MatchNetworkThread->WaitForCompletion(); // 스레드가 안전하게 끝날 때까지 기다립니다.
        delete MatchNetworkThread;
        MatchNetworkThread = nullptr;
    }

    if (MatchNetworkRunnable)
    {
        delete MatchNetworkRunnable;
        MatchNetworkRunnable = nullptr;
    }
}
// MainLobbyUserWidget.cpp 하단 어딘가에 추가

void UMainLobbyUserWidget::NativeDestruct()
{
    // 위젯이 파괴될 때 (창을 닫거나 맵을 이동할 때) 실행됩니다.
    // 백그라운드에 돌아가고 있는 스레드와 소켓을 깔끔하게 죽입니다.
    StopNetworkThread();

    if (MatchSocket)
    {
        MatchSocket->Close();
        MatchSocket = nullptr;
    }

    Super::NativeDestruct(); // 부모 클래스의 Destruct 호출 (필수)
}