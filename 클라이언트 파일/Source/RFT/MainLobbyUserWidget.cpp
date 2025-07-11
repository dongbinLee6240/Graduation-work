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

// Runnable Ŭ���� ����
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
        int RecvCount = 0; // recv ȣ�� Ƚ�� ����

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

                    if (BytesRead > 0) // ���� �����Ͱ� �ִ� ���
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
                            FPlatformProcess::Sleep(10.0f); // �� �����͸� �������� �� ��� (100ms)
                        }
                    }
                    else // `Recv` ���������� ���� �����Ͱ� ���� ��
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Recv succeeded but no data received. Adding sleep."));
                        FPlatformProcess::Sleep(10.0f); // �� ������ ó�� �� ���
                    }
                }
                else // `Recv` �������� ���
                {
                    UE_LOG(LogTemp, Warning, TEXT("Recv failed. Adding longer sleep."));
                    FPlatformProcess::Sleep(1.0f); // Recv ���� �� �� �� ��� (1��)
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


// MainLobbyUserWidget ��� ���� �ʱ�ȭ
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

    // OpenLevel �Լ��� �ش� �ּҷ� ������ �õ��մϴ�. �Ϲ������� 7777�� �⺻ ��Ʈ�Դϴ�.
    FString Command = FString::Printf(TEXT("%s:%d"), *ServerAddress, 7777);

    // 127.0.0.1 ������ ���� (��: ��Ƽ�÷��̾� ����)
    UGameplayStatics::OpenLevel(this, FName(*Command), true);
}

void UMainLobbyUserWidget::OnMatchButtonClicked()
{
    if (ConnectToServer())
    {
        TArray<uint8> Packet;
        CreateMatchRequestPacket(Packet);

        // MatchNetworkRunnable �ʱ�ȭ Ȯ��
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

        // ��Ŷ ���� Ȯ��
        if (MatchNetworkRunnable && MatchNetworkRunnable->SendMatchRequest(Packet))
        {
            UE_LOG(LogTemp, Log, TEXT("Match request sent successfully."));
            StartNetworkThread(); // ��Ʈ��ũ ������ ����
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
    // ����ŷ ���� ����
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
