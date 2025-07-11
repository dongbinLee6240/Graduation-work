// Fill out your copyright notice in the Description page of Project Settings.

#include "CreateCharacterUserWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "RFTGameInstance.h"
#include "Components/Image.h"    // UImage ������Ʈ
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"    // UTexture2D �ؽ�ó

#include "Http.h"
#include "Json.h"
#include "JsonUtilities.h"

void UCreateCharacterUserWidget::NativeConstruct()
{
	if (confirm_button)
	{
		confirm_button->OnClicked.AddDynamic(this, &UCreateCharacterUserWidget::OnConfirmButtonClicked);
	}

	if (knight_button)
	{
		knight_button->OnClicked.AddDynamic(this, &UCreateCharacterUserWidget::OnKnightButtonClicked);
	}

	if (magician_button)
	{
		magician_button->OnClicked.AddDynamic(this, &UCreateCharacterUserWidget::OnMagicianButtonClicked);
	}

	if (priest_button)
	{
		priest_button->OnClicked.AddDynamic(this, &UCreateCharacterUserWidget::OnPriestButtonClicked);
	}

	if (assassin_button)
	{
		assassin_button->OnClicked.AddDynamic(this, &UCreateCharacterUserWidget::OnAssassinButtonClicked);
    }

	if (archor_button)
	{
		archor_button->OnClicked.AddDynamic(this, &UCreateCharacterUserWidget::OnArchorButtonClicked);
	}
}

void UCreateCharacterUserWidget::OnConfirmButtonClicked()
{
	FString EnteredName = Character_Name->GetText().ToString();

	UE_LOG(LogTemp, Warning, TEXT("character_name: %s"),*EnteredName);

	SendConfirmRequest(EnteredName, SelectedClass);
}

void UCreateCharacterUserWidget::OnKnightButtonClicked()
{
	UTexture2D* KnightTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Images/knight_image.knight_image"));

	if (Class_image && KnightTexture)
	{
		Class_image->SetBrushFromTexture(KnightTexture); // �ؽ�ó ����
	}

	Show_Class->SetText(FText::FromString("Knight"));
	explain_class_title->SetText(FText::FromString("Knight"));
	Explain_text->SetText(FText::FromString("A strong melee fighter."));
	SelectedClass = "Knight";
	/*sex = "male";
	level = 0;*/
}

void UCreateCharacterUserWidget::OnMagicianButtonClicked()
{

	UTexture2D* MagicianTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Images/magician_image.magician_image"));

	if (Class_image && MagicianTexture)
	{
		Class_image->SetBrushFromTexture(MagicianTexture); // �ؽ�ó ����
	}

	Show_Class->SetText(FText::FromString("Magician"));
	explain_class_title->SetText(FText::FromString("Magician"));
	Explain_text->SetText(FText::FromString("Use Mana and Magic."));
	SelectedClass = "Magiciain";
	/*sex = "male";
	level = 0;*/
}

void UCreateCharacterUserWidget::OnPriestButtonClicked()
{
	UTexture2D* PriestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Images/priest_image.priest_image"));

	if (Class_image && PriestTexture)
	{
		Class_image->SetBrushFromTexture(PriestTexture); // �ؽ�ó ����
	}

	Show_Class->SetText(FText::FromString("Priest"));
	explain_class_title->SetText(FText::FromString("Priest"));
	Explain_text->SetText(FText::FromString("Use Holic Magic."));
	SelectedClass = "Priest";
	/*sex = "female";
	level = 0;*/
}

void UCreateCharacterUserWidget::OnAssassinButtonClicked()
{
	UTexture2D* AssassinTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Images/assassin_image.assassin_image"));

	if (Class_image && AssassinTexture)
	{
		Class_image->SetBrushFromTexture(AssassinTexture); // �ؽ�ó ����
	}

	Show_Class->SetText(FText::FromString("Assassin"));
	explain_class_title->SetText(FText::FromString("Assassin"));
	Explain_text->SetText(FText::FromString("rogue."));
	SelectedClass = "Assassin";
	/*sex = "male";
	level = 0;*/
}

void UCreateCharacterUserWidget::OnArchorButtonClicked()
{
	UTexture2D* ArchorTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Images/archor_image.archor_image"));

	if (Class_image && ArchorTexture)
	{
		Class_image->SetBrushFromTexture(ArchorTexture); // �ؽ�ó ����
	}
	
	Show_Class->SetText(FText::FromString("Archor"));
	explain_class_title->SetText(FText::FromString("Archor"));
	Explain_text->SetText(FText::FromString("Archor."));
	SelectedClass = "Archor";
	/*sex = "female";
	level = 0;*/
}

void UCreateCharacterUserWidget::SendConfirmRequest(FString name,FString class_type)
{
	FHttpModule* Http = &FHttpModule::Get();

	// RFTGameInstance���� UID ��������
	URFTGameInstance* GameInstance = Cast<URFTGameInstance>(UGameplayStatics::GetGameInstance(this));
	FString UID;
	if (GameInstance)
	{
		UID = GameInstance->GetUID();  // UID�� �����ɴϴ�.
	}

	// HTTP ��û ���� (POST)
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(TEXT("http://localhost:3000/character"));
	Request->SetVerb(TEXT("POST"));  // POST ������� ��û
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// JSON ������ ����
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("uid", UID);  // UID �߰�
	JsonObject->SetStringField("charactername", name);
	JsonObject->SetStringField("class_type", class_type);
	/*JsonObject->SetStringField("sex", c_sex);
	JsonObject->SetNumberField("level", c_level);*/

	// JSON�� ���ڿ��� ��ȯ
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// ��û ������ JSON ������ ����
	Request->SetContentAsString(RequestBody);

	// ��û �Ϸ� �� �ݹ� �Լ� ����
	Request->OnProcessRequestComplete().BindUObject(this, &UCreateCharacterUserWidget::OnConfirmResponseReceived);

	// ��û ����
	Request->ProcessRequest();
}

void UCreateCharacterUserWidget::OnConfirmResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response->GetResponseCode() == 200)
	{
		UGameplayStatics::OpenLevel(this, FName("SelectCharacter"));
		UE_LOG(LogTemp, Warning, TEXT("confirm Success: %s"), *Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("confirm Failed: %s"), *Response->GetContentAsString());
	}
}
