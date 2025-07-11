// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectCharacterUserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h" 
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"
#include "RFTGameInstance.h"

//현재 접속되어 있는 계정의 캐릭터를 화면에 표시(character_list1,character_list2)
void USelectCharacterUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

	if (enter_button)
	{
		enter_button->OnClicked.AddDynamic(this, &USelectCharacterUserWidget::OnEnterButtonClicked);
	}

    if (character_list1)
    {
        character_list1->OnClicked.AddDynamic(this, &USelectCharacterUserWidget::Oncharacter_list1ButtonClicked);
    }

    if (character_list2)
    {
        character_list2->OnClicked.AddDynamic(this, &USelectCharacterUserWidget::Oncharacter_list2ButtonClicked);
    }

    if (character_list3)
    {
        character_list3->OnClicked.AddDynamic(this, &USelectCharacterUserWidget::Oncharacter_list3ButtonClicked);
    }

    if (create_character_button)
    {
        create_character_button->OnClicked.AddDynamic(this, &USelectCharacterUserWidget::OnCreateCharacterButtonClicked);
    }

    GetCharacterList();
}

void USelectCharacterUserWidget::OnEnterButtonClicked()
{
    UGameplayStatics::OpenLevel(this, FName("MainLobby"));
}

void USelectCharacterUserWidget::GetCharacterList()
{
    FHttpModule* Http = &FHttpModule::Get();
    URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
    FString UID;
    if (GameInstance)
    {
        UID = GameInstance->GetUID();
        UE_LOG(LogTemp, Warning, TEXT("UID Retrieved: %s"), *UID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance is null or UID is missing."));
        return;
    }

    // HTTP 요청 생성
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
    Request->SetURL(TEXT("http://localhost:3000/get_characters"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // JSON 데이터 생성
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField("uid", UID); // UID 추가

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // 요청 본문에 JSON 데이터 설정
    Request->SetContentAsString(RequestBody);

    // 요청 완료 후 콜백 함수 설정
    Request->OnProcessRequestComplete().BindUObject(this, &USelectCharacterUserWidget::OnCharacterListResponseReceived);

    // 요청 전송
    Request->ProcessRequest();
}

void USelectCharacterUserWidget::Oncharacter_list1ButtonClicked()
{
    if (CIDArray.Num() > 0)  // CIDArray에 값이 있는지 확인
    {
        URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
        if (GameInstance)
        {
            GameInstance->SetCID(CIDArray[0]);  // 첫 번째 캐릭터 ID를 GameInstance에 저장
            UE_LOG(LogTemp, Warning, TEXT("Selected CID: %s"), *CIDArray[0]);
        }
    }
}

void USelectCharacterUserWidget::Oncharacter_list2ButtonClicked()
{
    if (CIDArray.Num() > 1)  // CIDArray에 character_id가 2개 이상 있는지 확인
    {
        URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
        if (GameInstance)
        {
            GameInstance->SetCID(CIDArray[1]);  // 2번째 캐릭터 ID를 GameInstance에 저장
            UE_LOG(LogTemp, Warning, TEXT("Selected CID: %s"), *CIDArray[0]);
        }
    }
}

void USelectCharacterUserWidget::Oncharacter_list3ButtonClicked()
{
    if (CIDArray.Num() > 2)  // CIDArray에 character_id가 3개 이상 있는지 확인
    {
        URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
        if (GameInstance)
        {
            GameInstance->SetCID(CIDArray[2]);  // 3번째 캐릭터 ID를 GameInstance에 저장
            UE_LOG(LogTemp, Warning, TEXT("Selected CID: %s"), *CIDArray[0]);
        }
    }
}

void USelectCharacterUserWidget::OnCreateCharacterButtonClicked()
{
    UGameplayStatics::OpenLevel(this, FName("CreateCharacter"));
}

void USelectCharacterUserWidget::OnCharacterListResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    CIDArray.Empty();
    if (bWasSuccessful && Response->GetResponseCode() == 200)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            const TArray<TSharedPtr<FJsonValue>> Characters = JsonObject->GetArrayField("characters");
            for (int32 i = 0; i < Characters.Num(); i++)
            {
                TSharedPtr<FJsonObject> CharacterObject = Characters[i]->AsObject();
                FString CharacterName = CharacterObject->GetStringField("charactername");
                FString CharacterClass = CharacterObject->GetStringField("class");
                FString CharacterID = CharacterObject->GetStringField("character_id");

                //CID 배열에 character_id 저장
                CIDArray.Add(CharacterID);

                // 캐릭터 정보를 버튼의 텍스트 변환
                if (i == 0)
                {
                    char_1->SetText(FText::FromString(FString::Printf(TEXT("%s (%s)"), *CharacterName, *CharacterClass)));

                }
                else if (i == 1)
                {
                    char_2->SetText(FText::FromString(FString::Printf(TEXT("%s (%s)"), *CharacterName, *CharacterClass)));
                }
                else if (i == 2)
                {
                    char_3->SetText(FText::FromString(FString::Printf(TEXT("%s (%s)"), *CharacterName, *CharacterClass)));
                }
                else if (i == 3)
                {
                    char_4->SetText(FText::FromString(FString::Printf(TEXT("%s (%s)"), *CharacterName, *CharacterClass)));
                }
                else if (i == 4)
                {
                    char_5->SetText(FText::FromString(FString::Printf(TEXT("%s (%s)"), *CharacterName, *CharacterClass)));
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to fetch character list: %s"), *Response->GetContentAsString());
    }
}
