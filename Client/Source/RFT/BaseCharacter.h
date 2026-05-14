// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h" // GAS 인터페이스
#include "BaseCharacter.generated.h"

class UAbilitySystemComponent;
class UBaseAttributeSet; // 나중에 만들 스탯(체력, 마나 등) 클래스

UCLASS()
class RFT_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();
	// IAbilitySystemInterface 필수 구현 함수
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 멀티플레이어 환경에서 서버 측 GAS 초기화
	virtual void PossessedBy(AController* NewController) override;

	// 멀티플레이어 환경에서 클라이언트 측 GAS 초기화
	virtual void OnRep_PlayerState() override;

	// GAS의 심장 역할을 하는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	// 캐릭터의 스탯(HP, MP 등)을 관리하는 컴포넌트
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	//UBaseAttributeSet* AttributeSet;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// IAbilitySystemInterface 필수 구현 함수

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
