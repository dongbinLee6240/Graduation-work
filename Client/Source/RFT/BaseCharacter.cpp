// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "Base_AttributeSet.h" // 다음 단계에서 만들 클래스이므로 일단 주석 처리

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// 기본적으로 틱(Tick)을 켭니다. 필요 없다면 false로 꺼주세요.
	PrimaryActorTick.bCanEverTick = true;

	// 멀티플레이어 게임이므로 액터 복제(Replication)를 반드시 켭니다.
	bReplicates = true;

	// 1. Ability System Component (ASC) 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// 플레이어가 조종하는 캐릭터는 'Mixed' 모드를 권장합니다. (서버 연산 결과를 클라이언트가 부드럽게 예측)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UBase_AttributeSet>(TEXT("AttributeSet"));

}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// [서버 측 초기화] 캐릭터가 컨트롤러(플레이어나 AI)에 빙의될 때 호출됨
void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		// InitAbilityActorInfo(소유자 액터, 아바타 액터)
		// GAS에게 "이 캐릭터가 시스템의 주인이다"라고 알려주는 핵심 함수입니다.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

// [클라이언트 측 초기화] 서버로부터 PlayerState가 복제되어 클라이언트에 도착했을 때 호출됨
void ABaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AbilitySystemComponent)
	{
		// 클라이언트도 동일하게 자신의 GAS를 초기화해줍니다.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

