// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EntryGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class RFT_API AEntryGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override; 

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UMG Game")
	UUserWidget* CurrentWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UMG Game")
	TSubclassOf<UUserWidget> HUDWidget;
	
};
