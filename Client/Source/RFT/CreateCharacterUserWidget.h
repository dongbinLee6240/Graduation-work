
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Http.h"

#include "CreateCharacterUserWidget.generated.h"


/**
 * 
 */
UCLASS()
class RFT_API UCreateCharacterUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;
	FString SelectedClass;

	/*FString sex;

	int32 level;*/

	UPROPERTY(meta =(BindWidget))

	class UTextBlock* Show_Class;

	UPROPERTY(meta =(BindWidget))

	class UTextBlock* explain_class_title;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Explain_text;

	UPROPERTY(meta=(BindWidget))

	class UEditableText* Character_Name;

	UPROPERTY(meta=(BindWidget))
	class UImage* Class_image;	

	UPROPERTY(meta = (BindWidget))

	class UButton* confirm_button;

	UPROPERTY(meta = (BindWidget))

	class UButton* knight_button;

	UPROPERTY(meta = (BindWidget))

	class UButton* magician_button;

	UPROPERTY(meta = (BindWidget))

	class UButton* priest_button;

	UPROPERTY(meta = (BindWidget))

	class UButton* assassin_button;

	UPROPERTY(meta=(BindWidget))
	class UButton* archor_button;
	UFUNCTION()
	void OnConfirmButtonClicked();

	UFUNCTION()
	void OnKnightButtonClicked();

	UFUNCTION()

	void OnMagicianButtonClicked();

	UFUNCTION()

	void OnPriestButtonClicked();

	UFUNCTION()

	void OnAssassinButtonClicked();

	UFUNCTION()

	void OnArchorButtonClicked();

	UFUNCTION()

	void SendConfirmRequest(FString name,FString class_type);

	void OnConfirmResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
