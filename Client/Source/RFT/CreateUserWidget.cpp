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
		// 블루프린트로 만든 CreateUI를 불러옵니다.
		UUserWidget* CloseWidget = CreateWidget<UUserWidget>(World, BackWidget);
		if (CloseWidget)
		{
			// 새 위젯을 화면에 추가합니다.
			CloseWidget->AddToViewport();
		}
	}
}

void UCreateUserWidget::CreateAccountRequest(FString name, FString ID, FString Password)
{
    FHttpModule* Http = &FHttpModule::Get();

    // HTTP 요청 생성 (POST)
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
    Request->SetURL(TEXT("http://localhost:3000/register"));  // 회원가입 엔드포인트로 URL 설정
    Request->SetVerb(TEXT("POST"));  // POST 방식으로 요청
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // JSON 데이터 생성
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField("username", name);
    JsonObject->SetStringField("id", ID);
    JsonObject->SetStringField("password", Password);

    // JSON을 문자열로 변환
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // 요청 본문에 JSON 데이터 설정
    Request->SetContentAsString(RequestBody);

    // 요청 완료 후 콜백 함수 설정
    Request->OnProcessRequestComplete().BindUObject(this, &UCreateUserWidget::OnRegisterResponseReceived);

    // 요청 전송
    Request->ProcessRequest();
}

void UCreateUserWidget::OnRegisterResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response->GetResponseCode() == 200)
    {
        // 회원가입 성공
        UE_LOG(LogTemp, Warning, TEXT("Registration Success: %s"), *Response->GetContentAsString());
    }
    else
    {
        // 회원가입 실패
        UE_LOG(LogTemp, Error, TEXT("Registration Failed: %s"), *Response->GetContentAsString());
    }
}

