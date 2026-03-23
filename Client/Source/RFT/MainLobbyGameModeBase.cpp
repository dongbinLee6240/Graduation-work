// Fill out your copyright notice in the Description page of Project Settings.


#include "MainLobbyGameModeBase.h"
#include "Blueprint/UserWidget.h"

void AMainLobbyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	CurrWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidget);
	CurrWidget->AddToViewport();
}
