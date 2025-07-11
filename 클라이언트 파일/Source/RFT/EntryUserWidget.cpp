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

	// 로그인 성공 시 다음 레벨로 전환하는 코드
	//if (IsLoginSuccessful(EnteredID, EnteredPassword)) // 여기에 실제 로그인 성공 여부를 확인하는 로직이 필요합니다.
	//{
	/*}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Login Failed"));
	}*/
}

void UEntryUserWidget::OnCreateButtonClicked()
{
	// 현재 EntryUserWidget을 닫습니다.
	RemoveFromParent();

	// UWorld 객체가 유효한지 확인합니다.
	if (UWorld* World = GetWorld())
	{
		// 블루프린트로 만든 CreateUI를 불러옵니다.
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(World, CreateWidgetClass);
		if (CreatedWidget)
		{
			// 새 위젯을 화면에 추가합니다.
			CreatedWidget->AddToViewport();
		}
	}
}

//Login은 EntryUserWidget에서 Create는 CreateUserWidget에서
void UEntryUserWidget::SendLoginRequest(FString ID, FString PassWord)
{
	FHttpModule* Http = &FHttpModule::Get();

	// HTTP 요청 생성 (POST)
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(TEXT("http://localhost:3000/login"));  // 로그인 엔드포인트로 URL 설정
	Request->SetVerb(TEXT("POST"));  // POST 방식으로 요청
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// JSON 데이터 생성
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("id", ID);  // 로그인용 필드
	JsonObject->SetStringField("password", PassWord);

	// JSON을 문자열로 변환
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// 요청 본문에 JSON 데이터 설정
	Request->SetContentAsString(RequestBody);

	// 요청 완료 후 콜백 함수 설정
	Request->OnProcessRequestComplete().BindUObject(this, &UEntryUserWidget::OnLoginResponseReceived);

	// 요청 전송
	Request->ProcessRequest();
}

void UEntryUserWidget::OnLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response->GetResponseCode() == 200)
	{
		// JSON 파싱을 통해 UID를 추출
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString OutputString;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

			// JsonObject를 JSON 형식의 문자열로 변환
			FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

			// 변환된 JSON 문자열을 출력
			UE_LOG(LogTemp, Warning, TEXT("JsonObject: %s"), *OutputString);

			FString UID = JsonObject->GetStringField("uid");  // 서버에서 UID를 받아오기
			bool HasCharacter = JsonObject->GetBoolField("hasCharacter");  // 캐릭터 존재 여부

			// RFTGameInstance에 UID 저장
			URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
			if (GameInstance)
			{
				GameInstance->SetUID(UID);
				UE_LOG(LogTemp, Warning, TEXT("Login Success. UID: %s"), *UID);
			}

			// 캐릭터가 존재하는지 여부에 따라 다른 레벨로 전환
			if (HasCharacter)
			{
				UGameplayStatics::OpenLevel(this, FName("SelectCharacter"));  // 캐릭터가 있으면 SelectCharacter 레벨로 이동
			}
			else
			{
				UGameplayStatics::OpenLevel(this, FName("CreateCharacter"));  // 없으면 CreateCharacter 레벨로 이동
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON response"));
		}
	}
	else
	{
		// 로그인 실패
		UE_LOG(LogTemp, Error, TEXT("Login Failed: %s"), *Response->GetContentAsString());
	}
}
