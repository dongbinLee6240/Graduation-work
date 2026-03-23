// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"

#include "CreateUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class RFT_API UCreateUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> BackWidget;
private:

	UPROPERTY(meta = (BindWidget))
	class UButton* Create_Button;

	UPROPERTY(meta = (BindWidget))

	class UButton* Close_Button;

	UPROPERTY(meta = (BindWidget))
	class UEditableText* EditName;

	UPROPERTY(meta = (BindWidget))  
		class UEditableText* EditId;

	UPROPERTY(meta = (BindWidget))  
		class UEditableText* EditPassword;

	UFUNCTION()

	void CreateAccount();
	
	UFUNCTION()
	void CloseUI();

	UFUNCTION()

	void CreateAccountRequest(FString name, FString id, FString password);

	void OnRegisterResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
