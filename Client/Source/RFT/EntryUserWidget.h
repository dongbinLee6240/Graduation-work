#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"

#include "EntryUserWidget.generated.h"  // 항상 마지막에 위치해야 함

/**
 *
 */
UCLASS()
class RFT_API UEntryUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// CreateWidgetClass가 블루프린트로 만든 CreateUI 위젯을 참조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> CreateWidgetClass;
private:

	UPROPERTY(meta = (BindWidget))
	class UButton* Login_Button;

	UPROPERTY(meta = (BindWidget))
	class UButton* Create_Button;

	UPROPERTY(meta = (BindWidget))  
		class UEditableText* EditId;

	UPROPERTY(meta = (BindWidget))  
		class UEditableText* EditPassword;
	UFUNCTION()
	void OnLoginButtonClicked();

	UFUNCTION()
	void OnCreateButtonClicked(); //Create를 누를 시 현재의 로그인 UI가 꺼지고 CreateUI가 나오게 하기

	UFUNCTION()
	void SendLoginRequest(FString id, FString password);

	void OnLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
