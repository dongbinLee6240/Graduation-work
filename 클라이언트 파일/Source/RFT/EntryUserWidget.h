#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"

#include "EntryUserWidget.generated.h"  // �׻� �������� ��ġ�ؾ� ��

/**
 *
 */
UCLASS()
class RFT_API UEntryUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// CreateWidgetClass�� �������Ʈ�� ���� CreateUI ������ ����
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
	void OnCreateButtonClicked(); //Create�� ���� �� ������ �α��� UI�� ������ CreateUI�� ������ �ϱ�

	UFUNCTION()
	void SendLoginRequest(FString id, FString password);

	void OnLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
