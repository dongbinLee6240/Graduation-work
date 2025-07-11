#include "MainLobbyUserWidget.h"
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
        UE_LOG(LogTemp, Warning, TEXT("Run Start"));
        int RecvCount = 0; // recv 호출 횟수 추적

        while (bIsRunning)
        {
            UE_LOG(LogTemp, Warning, TEXT("while loop start"));

            if (MatchSocket && MatchSocket->GetConnectionState() == SCS_Connected)
            {
                TArray<uint8> ReceiveBuffer;
                ReceiveBuffer.SetNum(1024);
                int32 BytesRead = 0;

                if (MatchSocket->Recv(ReceiveBuffer.GetData(), ReceiveBuffer.Num(), BytesRead))
                {
                    RecvCount++;
                    UE_LOG(LogTemp, Warning, TEXT("Recv called %d times. Bytes read: %d"), RecvCount, BytesRead);

                    if (BytesRead > 0) // 실제 데이터가 있는 경우
                    {
                        FString Data = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceiveBuffer.GetData() + 2)));
                        uint8 Header = ReceiveBuffer[2];

                        if (!Data.IsEmpty() && OnResponseReceived)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Invoking OnResponseReceived. Header: %d, Data: %s"), Header, *Data);
                            OnResponseReceived(Data, Header);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("Received empty data. Skipping HandleServerResponse."));
                            FPlatformProcess::Sleep(10.0f); // 빈 데이터를 수신했을 때 대기 (100ms)
                        }
                    }
                    else // `Recv` 성공했으나 실제 데이터가 없을 때
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Recv succeeded but no data received. Adding sleep."));
                        FPlatformProcess::Sleep(10.0f); // 빈 데이터 처리 후 대기
                    }
                }
                else // `Recv` 실패했을 경우
                {
                    UE_LOG(LogTemp, Warning, TEXT("Recv failed. Adding longer sleep."));
                    FPlatformProcess::Sleep(1.0f); // Recv 실패 시 더 긴 대기 (1초)
                }
            }
            else if (MatchSocket->GetConnectionState() != SCS_Connected)
            {
                UE_LOG(LogTemp, Error, TEXT("Socket disconnected during Recv loop."));
                break;
            }

            if (!bIsRunning)
            {
                UE_LOG(LogTemp, Warning, TEXT("bIsRunning is false. Exiting Recv loop."));
                break;
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
    FString CID;
    URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (GameInstance)
    {
        CID = GameInstance->GetCID();
    }

    uint8 Header = static_cast<uint8>(EHeaderType::REQ_MATCH);
    unsigned char End = 0xff;

    int32 PacketLength = sizeof(Header) + CID.Len() + sizeof(End);
    Packet.SetNum(PacketLength);

    int32 Offset = 0;
    FMemory::Memcpy(Packet.GetData() + Offset, &Header, sizeof(Header));
    Offset += sizeof(Header);

    FMemory::Memcpy(Packet.GetData() + Offset, TCHAR_TO_ANSI(*CID), CID.Len());
    Offset += CID.Len();

    Packet[Offset] = End;

    UE_LOG(LogTemp, Warning, TEXT("Created req_match packet with CID: %s"), *CID);
}

void UMainLobbyUserWidget::HandleServerResponse(const FString& Data, uint8 Header)
{
    UE_LOG(LogTemp, Warning, TEXT("Recv packet data: %s"), *Data);
    switch (static_cast<EHeaderType>(Header))
    {

    case EHeaderType::MATCH_COMPLETE:
        UE_LOG(LogTemp, Warning, TEXT("Match completed: %s"), *Data);
        Mcomp = true;
        StopNetworkThread();
        FString ServerAddress = TEXT("127.0.0.1");
        FString Command = FString::Printf(TEXT("%s:%d"), *ServerAddress, 7777);
        UGameplayStatics::OpenLevel(this, FName(*Command), true);
     
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
