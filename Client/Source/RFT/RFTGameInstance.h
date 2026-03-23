// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RFTGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class RFT_API URFTGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void SetUID(const FString& NewUID);

	void SetCID(const FString& NewCID);

	//void SetClass_type(const FString& SelectedClass);

	// UID를 가져오는 함수
	FString GetCID() const;
	FString GetUID() const;
	//character_id;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FString UID;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)

	FString CID;
	
};
