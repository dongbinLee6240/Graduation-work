// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectCharacterGameModeBase.h"
#include "Blueprint/UserWidget.h"

void ASelectCharacterGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	CurrWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidget);
	CurrWidget->AddToViewport();
}
