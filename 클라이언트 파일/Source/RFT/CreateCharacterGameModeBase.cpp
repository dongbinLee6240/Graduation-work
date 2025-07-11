// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateCharacterGameModeBase.h"
#include "Blueprint/UserWidget.h"

void ACreateCharacterGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	CurrWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidget);
	CurrWidget->AddToViewport();
}
