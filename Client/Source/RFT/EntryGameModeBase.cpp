// Fill out your copyright notice in the Description page of Project Settings.


#include "EntryGameModeBase.h"
#include "Blueprint/UserWidget.h"

void AEntryGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidget);
	CurrentWidget->AddToViewport();
}
