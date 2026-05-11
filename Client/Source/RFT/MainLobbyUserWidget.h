// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
// Sockets.h 와 SocketSubsystem.h는 cpp 파일로 이동하여 컴파일 속도를 최적화합니다.
#include "MainLobbyUserWidget.generated.h"

// (참고: 서버와 Protobuf를 사용 중이시므로, 나중에는 이 Enum 대신 Protobuf의 MessageId를 그대로 쓰셔도 됩니다.)
UENUM(BlueprintType)
enum class EHeaderType : uint8
{
    REQ_MATCH = 0 UMETA(DisplayName = "Request Match"),
    MATCHING = 1 UMETA(DisplayName = "MATCHING"),
    RES_MATCH_SUCCESS = 2 UMETA(DisplayName = "MATCH_COMPLETE")
};

// 전방 선언 (헤더 파일 최적화)
class FSocket;
class FMatchNetworkRunnable;
class FRunnableThread;
class UButton;

/**
 * Main Lobby User Widget
 */
UCLASS()
class RFT_API UMainLobbyUserWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    // [추가] 위젯이 화면에서 사라질 때 스레드와 소켓을 안전하게 끄기 위해 오버라이드합니다.
    virtual void NativeDestruct() override;

    // UI 버튼 바인딩
    UPROPERTY(meta = (BindWidget))
    UButton* matching;

    UPROPERTY(meta = (BindWidget))
    UButton* Join;

private:
    // 상태 변수
    bool ConnectStatus = false;
    bool Mcomp = false;
    // (RecvTimerHandle은 스레드 도입으로 인해 삭제)

    // 네트워크 및 스레드 객체 (글로벌 변수 대신 멤버 변수로 안전하게 관리)
    FSocket* MatchSocket = nullptr;
    FMatchNetworkRunnable* MatchNetworkRunnable = nullptr;
    FRunnableThread* MatchNetworkThread = nullptr;

    // 블루프린트(UI) 버튼 이벤트와 직접 연결되는 함수들만 UFUNCTION()을 유지합니다.
    UFUNCTION()
    void OnJoinButtonClicked();

    UFUNCTION()
    void OnMatchButtonClicked();

    // 내부 C++ 로직 전용 함수들 (UHT 에러 방지를 위해 UFUNCTION 제거)
    bool ConnectToServer();
    void CreateMatchRequestPacket(TArray<uint8>& Packet);
    void HandleServerResponse(const TArray<uint8>& Data, uint8 Header);
    void StartNetworkThread();
    void StopNetworkThread();
};