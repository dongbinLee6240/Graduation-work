// Fill out your copyright notice in the Description page of Project Settings.


#include "Base_AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h" // PostGameplayEffectExecute 처리를 위해 필수
#include "GameFramework/Character.h"

UBase_AttributeSet::UBase_AttributeSet()
{
	// 기획하신 기본 공통 수치 초기화 (직업별 수치는 나중에 데이터로 덮어씌웁니다)
	InitStamina(100.0f);
	InitCritRate(0.0f);         // 0%
	InitCritMultiplier(0.0f);   // 0배
	InitAttackSpeed(1.0f);      // 기준값 1.0
	InitCooldownReduction(0.0f); // 0%
	InitHealthRegen(0.0f);       //기본 자연 회복 없음
	InitManaRegen(0.0f);
}

void UBase_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 클라이언트들에게 변수 값의 변화를 알려줍니다 (메타 어트리뷰트인 Damage, Healing은 제외)
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, MaxHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, CurrentHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, HealthRegen, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, MaxMP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, CurrentMP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, ManaRegen, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, PhysicalAttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, MagicalAttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, PhysicalDefense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, MagicalDefense, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, CritRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, CritMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBase_AttributeSet, CooldownReduction, COND_None, REPNOTIFY_Always);
}

void UBase_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 예: CurrentHP가 변할 때 MaxHP를 넘지 못하도록 클램핑(Clamping)
	if (Attribute == GetCurrentHPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
	}
	else if (Attribute == GetCurrentMPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMP());
	}
}

//  메타 어트리뷰트(Damage, Healing)가 작동하는 핵심 서버 로직
void UBase_AttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 1. 피해(Damage) 처리가 들어왔을 때
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// 들어온 데미지 수치 저장
		const float LocalDamageDone = GetDamage();

		// 데미지 스탯 초기화 (다음 공격을 위해 비워둠)
		SetDamage(0.0f);

		if (LocalDamageDone > 0.0f)
		{
			// 현재 체력에서 데미지를 차감하고 0 미만으로 떨어지지 않게 고정
			const float NewHP = FMath::Clamp(GetCurrentHP() - LocalDamageDone, 0.0f, GetMaxHP());
			SetCurrentHP(NewHP);

			// TODO: NewHP가 0이라면 여기서 "사망(Death)" 로직이나 이벤트를 발생시킵니다.
		}
	}
	// 2. 회복(Healing) 처리가 들어왔을 때
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		const float LocalHealingDone = GetHealing();
		SetHealing(0.0f);

		if (LocalHealingDone > 0.0f)
		{
			// 현재 체력에 회복량을 더하고 MaxHP를 넘지 않게 고정
			const float NewHP = FMath::Clamp(GetCurrentHP() + LocalHealingDone, 0.0f, GetMaxHP());
			SetCurrentHP(NewHP);
		}
	}
}

/* ==========================================
 * OnRep 콜백 구현부 (네트워크 환경에서 값 갱신 시 실행됨)
 * ========================================== */
void UBase_AttributeSet::OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, MaxHP, OldMaxHP); }
void UBase_AttributeSet::OnRep_CurrentHP(const FGameplayAttributeData& OldCurrentHP) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, CurrentHP, OldCurrentHP); }
void UBase_AttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, HealthRegen, OldHealthRegen); }
void UBase_AttributeSet::OnRep_MaxMP(const FGameplayAttributeData& OldMaxMP) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, MaxMP, OldMaxMP); }
void UBase_AttributeSet::OnRep_CurrentMP(const FGameplayAttributeData& OldCurrentMP) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, CurrentMP, OldCurrentMP); }
void UBase_AttributeSet::OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, ManaRegen, OldManaRegen); }

void UBase_AttributeSet::OnRep_PhysicalAttackPower(const FGameplayAttributeData& OldPhysicalAttackPower) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, PhysicalAttackPower, OldPhysicalAttackPower); }
void UBase_AttributeSet::OnRep_MagicalAttackPower(const FGameplayAttributeData& OldMagicalAttackPower) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, MagicalAttackPower, OldMagicalAttackPower); }
void UBase_AttributeSet::OnRep_PhysicalDefense(const FGameplayAttributeData& OldPhysicalDefense) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, PhysicalDefense, OldPhysicalDefense); }
void UBase_AttributeSet::OnRep_MagicalDefense(const FGameplayAttributeData& OldMagicalDefense) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, MagicalDefense, OldMagicalDefense); }

void UBase_AttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, MoveSpeed, OldMoveSpeed); }
void UBase_AttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, Stamina, OldStamina); }
void UBase_AttributeSet::OnRep_CritRate(const FGameplayAttributeData& OldCritRate) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, CritRate, OldCritRate); }
void UBase_AttributeSet::OnRep_CritMultiplier(const FGameplayAttributeData& OldCritMultiplier) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, CritMultiplier, OldCritMultiplier); }
void UBase_AttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, AttackSpeed, OldAttackSpeed); }
void UBase_AttributeSet::OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction) { GAMEPLAYATTRIBUTE_REPNOTIFY(UBase_AttributeSet, CooldownReduction, OldCooldownReduction); }

