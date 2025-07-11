// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateUserWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

#include "Http.h"
#include "Json.h"
#include "JsonUtilities.h"

void UCreateUserWidget::NativeConstruct()
{
	if (Create_Button)
	{
		Create_Button->OnClicked.AddDynamic(this, &UCreateUserWidget::CreateAccount);
	}

	if (Close_Button)
	{
		Close_Button->OnClicked.AddDynamic(this, &UCreateUserWidget::CloseUI);
	}
}

void UCreateUserWidget::CreateAccount()
{
    FString EnteredName = EditName->GetText().ToString();
	FString EnteredID = EditId->GetText().ToString();
	FString EnteredPassword = EditPassword->GetText().ToString();
	UE_LOG(LogTemp, Warning, TEXT("Login Button Clicked"));
	UE_LOG(LogTemp, Warning, TEXT("Name: %s, ID: %s, Password: %s"),*EnteredName, *EnteredID, *EnteredPassword);

	CreateAccountRequest(EnteredName, EnteredID, EnteredPassword);
}

void UCreateUserWidget::CloseUI()
{
	RemoveFromParent();

	if (UWorld* World = GetWorld())
	{
		// �������Ʈ�� ���� CreateUI�� �ҷ��ɴϴ�.
		UUserWidget* CloseWidget = CreateWidget<UUserWidget>(World, BackWidget);
		if (CloseWidget)
		{
			// �� ������ ȭ�鿡 �߰��մϴ�.
			CloseWidget->AddToViewport();
		}
	}
}

void UCreateUserWidget::CreateAccountRequest(FString name, FString ID, FString Password)
{
    FHttpModule* Http = &FHttpModule::Get();

    // HTTP ��û ���� (POST)
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
    Request->SetURL(TEXT("http://localhost:3000/register"));  // ȸ������ ��������Ʈ�� URL ����
    Request->SetVerb(TEXT("POST"));  // POST ������� ��û
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // JSON ������ ����
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField("username", name);
    JsonObject->SetStringField("id", ID);
    JsonObject->SetStringField("password", Password);

    // JSON�� ���ڿ��� ��ȯ
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // ��û ������ JSON ������ ����
    Request->SetContentAsString(RequestBody);

    // ��û �Ϸ� �� �ݹ� �Լ� ����
    Request->OnProcessRequestComplete().BindUObject(this, &UCreateUserWidget::OnRegisterResponseReceived);

    // ��û ����
    Request->ProcessRequest();
}

void UCreateUserWidget::OnRegisterResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response->GetResponseCode() == 200)
    {
        // ȸ������ ����
        UE_LOG(LogTemp, Warning, TEXT("Registration Success: %s"), *Response->GetContentAsString());
    }
    else
    {
        // ȸ������ ����
        UE_LOG(LogTemp, Error, TEXT("Registration Failed: %s"), *Response->GetContentAsString());
    }
}

