// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "MainLobbyUserWidget.generated.h"

UENUM(BlueprintType) // Optional: Blueprint에서 사용 가능하게 설정
enum class EHeaderType : uint8
{
	REQ_MATCH = 0 UMETA(DisplayName = "Request Match"),
	MATCHING = 1 UMETA(DisplayName = "MATCHING"),
	MATCH_COMPLETE = 2 UMETA(DisplayName = "MATCH_COMPLETE")
};

/**
 * 
 */
UCLASS()
class RFT_API UMainLobbyUserWidget : public UUserWidget
{
	GENERATED_BODY()
	FSocket* MatchSocket;
	bool ConnectStatus = false;
	bool Mcomp = false;
	FTimerHandle RecvTimerHandle;

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))

	class UButton* matching;

	UPROPERTY(meta = (BindWidget))

	class UButton* Join;
	UFUNCTION()
	void OnJoinButtonClicked();

	UFUNCTION()
	void OnMatchButtonClicked();

	UFUNCTION()
	bool ConnectToServer();

	UFUNCTION()
	void CreateMatchRequestPacket(TArray<uint8>& Packet);

	UFUNCTION()
	void HandleServerResponse(const FString& Data, uint8 Header);

	UFUNCTION()
	void StartNetworkThread();

	UFUNCTION()
	void StopNetworkThread();
	/*UFUNCTION()
	void StartReceiving();

	UFUNCTION()
	void StopReceiving();*/

	
};
