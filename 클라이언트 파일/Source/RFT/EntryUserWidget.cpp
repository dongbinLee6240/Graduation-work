// Fill out your copyright notice in the Description page of Project Settings.


#include "EntryUserWidget.h"
#include "Components/Button.h"
#include "RFTGameInstance.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

#include "Http.h"
#include "Json.h"
#include "JsonUtilities.h"

void UEntryUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Create_Button)
	{
		Create_Button->OnClicked.AddDynamic(this, &UEntryUserWidget::OnCreateButtonClicked);
	}

	if (Login_Button)
	{
		Login_Button->OnClicked.AddDynamic(this, &UEntryUserWidget::OnLoginButtonClicked);
	}

}

void UEntryUserWidget::OnLoginButtonClicked()
{
	FString EnteredID = EditId->GetText().ToString();
	FString EnteredPassword = EditPassword->GetText().ToString();
	UE_LOG(LogTemp, Warning, TEXT("Login Button Clicked"));
	UE_LOG(LogTemp, Warning, TEXT("ID: %s, Password: %s"), *EnteredID, *EnteredPassword);

	SendLoginRequest(EnteredID,EnteredPassword);

	// �α��� ���� �� ���� ������ ��ȯ�ϴ� �ڵ�
	//if (IsLoginSuccessful(EnteredID, EnteredPassword)) // ���⿡ ���� �α��� ���� ���θ� Ȯ���ϴ� ������ �ʿ��մϴ�.
	//{
	/*}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Login Failed"));
	}*/
}

void UEntryUserWidget::OnCreateButtonClicked()
{
	// ���� EntryUserWidget�� �ݽ��ϴ�.
	RemoveFromParent();

	// UWorld ��ü�� ��ȿ���� Ȯ���մϴ�.
	if (UWorld* World = GetWorld())
	{
		// �������Ʈ�� ���� CreateUI�� �ҷ��ɴϴ�.
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(World, CreateWidgetClass);
		if (CreatedWidget)
		{
			// �� ������ ȭ�鿡 �߰��մϴ�.
			CreatedWidget->AddToViewport();
		}
	}
}

//Login�� EntryUserWidget���� Create�� CreateUserWidget����
void UEntryUserWidget::SendLoginRequest(FString ID, FString PassWord)
{
	FHttpModule* Http = &FHttpModule::Get();

	// HTTP ��û ���� (POST)
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(TEXT("http://localhost:3000/login"));  // �α��� ��������Ʈ�� URL ����
	Request->SetVerb(TEXT("POST"));  // POST ������� ��û
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// JSON ������ ����
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("id", ID);  // �α��ο� �ʵ�
	JsonObject->SetStringField("password", PassWord);

	// JSON�� ���ڿ��� ��ȯ
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// ��û ������ JSON ������ ����
	Request->SetContentAsString(RequestBody);

	// ��û �Ϸ� �� �ݹ� �Լ� ����
	Request->OnProcessRequestComplete().BindUObject(this, &UEntryUserWidget::OnLoginResponseReceived);

	// ��û ����
	Request->ProcessRequest();
}

void UEntryUserWidget::OnLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response->GetResponseCode() == 200)
	{
		// JSON �Ľ��� ���� UID�� ����
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString OutputString;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

			// JsonObject�� JSON ������ ���ڿ��� ��ȯ
			FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

			// ��ȯ�� JSON ���ڿ��� ���
			UE_LOG(LogTemp, Warning, TEXT("JsonObject: %s"), *OutputString);

			FString UID = JsonObject->GetStringField("uid");  // �������� UID�� �޾ƿ���
			bool HasCharacter = JsonObject->GetBoolField("hasCharacter");  // ĳ���� ���� ����

			// RFTGameInstance�� UID ����
			URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
			if (GameInstance)
			{
				GameInstance->SetUID(UID);
				UE_LOG(LogTemp, Warning, TEXT("Login Success. UID: %s"), *UID);
			}

			// ĳ���Ͱ� �����ϴ��� ���ο� ���� �ٸ� ������ ��ȯ
			if (HasCharacter)
			{
				UGameplayStatics::OpenLevel(this, FName("SelectCharacter"));  // ĳ���Ͱ� ������ SelectCharacter ������ �̵�
			}
			else
			{
				UGameplayStatics::OpenLevel(this, FName("CreateCharacter"));  // ������ CreateCharacter ������ �̵�
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON response"));
		}
	}
	else
	{
		// �α��� ����
		UE_LOG(LogTemp, Error, TEXT("Login Failed: %s"), *Response->GetContentAsString());
	}
}
