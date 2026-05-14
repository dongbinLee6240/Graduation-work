// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h" //GAS
#include "Base_AttributeSet.generated.h"

// GAS 전용 매크로: 속성에 대한 Getter, Setter, Init 함수를 
// 자동으로 생성해 줍니다.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
/**
 * 
 */
UCLASS()
class RFT_API UBase_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBase_AttributeSet();

	// 멀티플레이어 환경에서 변수 동기화를 위한 필수 등록 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 값이 변경되기 '직전'에 호출됨 (예: 최대 HP 이상으로 회복되지 않게 클램핑)
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// Gameplay Effect가 적용된 '직후'에 호출됨 (데미지, 힐링 등 메타 어트리뷰트 연산 처리)
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	/* ==========================================
	 * 생명력 (Health) & 마나 (Mana)
	 * ========================================== */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_MaxHP)
	FGameplayAttributeData MaxHP;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, MaxHP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_CurrentHP)
	FGameplayAttributeData CurrentHP;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, CurrentHP)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_HealthRegen)
	FGameplayAttributeData HealthRegen;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, HealthRegen)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mana", ReplicatedUsing = OnRep_MaxMP)
	FGameplayAttributeData MaxMP;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, MaxMP)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mana", ReplicatedUsing = OnRep_CurrentMP)
	FGameplayAttributeData CurrentMP;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, CurrentMP)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mana", ReplicatedUsing = OnRep_ManaRegen)
	FGameplayAttributeData ManaRegen;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, ManaRegen)

		/* ==========================================
		 * 공격 & 방어 (Offense & Defense)
		 * ========================================== */
		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Offense", ReplicatedUsing = OnRep_PhysicalAttackPower)
	FGameplayAttributeData PhysicalAttackPower;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, PhysicalAttackPower)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Offense", ReplicatedUsing = OnRep_MagicalAttackPower)
	FGameplayAttributeData MagicalAttackPower;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, MagicalAttackPower)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Defense", ReplicatedUsing = OnRep_PhysicalDefense)
	FGameplayAttributeData PhysicalDefense;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, PhysicalDefense)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Defense", ReplicatedUsing = OnRep_MagicalDefense)
	FGameplayAttributeData MagicalDefense;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, MagicalDefense)

		/* ==========================================
		 * 유틸리티 (Utility)
		 * ========================================== */
		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Utility", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, MoveSpeed)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Utility", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, Stamina)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_CritRate)
	FGameplayAttributeData CritRate;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, CritRate)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_CritMultiplier)
	FGameplayAttributeData CritMultiplier;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, CritMultiplier)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, AttackSpeed)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Utility", ReplicatedUsing = OnRep_CooldownReduction)
	FGameplayAttributeData CooldownReduction;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, CooldownReduction)

		/* ==========================================
		 * 메타 어트리뷰트 (Meta Attributes)
		 * 서버에서만 연산되므로 Replicate(동기화) 하지 않습니다.
		 * ========================================== */
		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, Damage)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UBase_AttributeSet, Healing)

protected:
	// 동기화(Replication) 콜백 함수들
	UFUNCTION() virtual void OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP);
	UFUNCTION() virtual void OnRep_CurrentHP(const FGameplayAttributeData& OldCurrentHP);
	UFUNCTION() virtual void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen);
	UFUNCTION() virtual void OnRep_MaxMP(const FGameplayAttributeData& OldMaxMP);
	UFUNCTION() virtual void OnRep_CurrentMP(const FGameplayAttributeData& OldCurrentMP);
	UFUNCTION() virtual void OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen);

	UFUNCTION() virtual void OnRep_PhysicalAttackPower(const FGameplayAttributeData& OldPhysicalAttackPower);
	UFUNCTION() virtual void OnRep_MagicalAttackPower(const FGameplayAttributeData& OldMagicalAttackPower);
	UFUNCTION() virtual void OnRep_PhysicalDefense(const FGameplayAttributeData& OldPhysicalDefense);
	UFUNCTION() virtual void OnRep_MagicalDefense(const FGameplayAttributeData& OldMagicalDefense);

	UFUNCTION() virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);
	UFUNCTION() virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
	UFUNCTION() virtual void OnRep_CritRate(const FGameplayAttributeData& OldCritRate);
	UFUNCTION() virtual void OnRep_CritMultiplier(const FGameplayAttributeData& OldCritMultiplier);
	UFUNCTION() virtual void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);
	UFUNCTION() virtual void OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction);
	
};
