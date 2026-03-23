// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <vector>
#include "Http.h"
#include "SelectCharacterUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class RFT_API USelectCharacterUserWidget : public UUserWidget
{
	GENERATED_BODY()
	TArray<FString> CIDArray;

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))

	class UImage* Show_Character;

	UPROPERTY(meta = (BindWidget))
	class UButton* enter_button;

	UPROPERTY(meta = (BindWidget))
	class UButton* character_list1;

	UPROPERTY(meta = (BindWidget))
	class UButton* character_list2;

	UPROPERTY(meta = (BindWidget))
	class UButton* character_list3;

	UPROPERTY(meta=(BindWidget))

	class UButton* create_character_button;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* char_1;

	UPROPERTY(meta =(BindWidget))
	class UTextBlock* char_2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* char_3;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* char_4;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* char_5;
	
	UFUNCTION()
	void OnEnterButtonClicked();

	UFUNCTION()
	void GetCharacterList();

	UFUNCTION()
	void Oncharacter_list1ButtonClicked();

	UFUNCTION()
	void Oncharacter_list2ButtonClicked();

	UFUNCTION()
	void Oncharacter_list3ButtonClicked();

	/*UFUNCTION()
	void Oncharacter_list4ButtonClicked();*/

	UFUNCTION()
	void OnCreateCharacterButtonClicked();

	void OnCharacterListResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
