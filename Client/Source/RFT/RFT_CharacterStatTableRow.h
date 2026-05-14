#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "RFT_CharacterStatTableRow.generated.h" 

// USTRUCT 매크로를 붙여야 블루프린트와 에디터(DataTable)에서 인식할 수 있습니다.
USTRUCT(BlueprintType)
struct FRFT_CharacterStatTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxMP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float PhysicalAttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MagicalAttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float PhysicalDefense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MagicalDefense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MoveSpeed;

	// 기본 생성자: 쓰레기값(Garbage Value)이 들어가는 것을 방지하는 필수 안전장치
	FRFT_CharacterStatTableRow()
		: MaxHP(0.0f), MaxMP(0.0f), PhysicalAttackPower(0.0f), MagicalAttackPower(0.0f), PhysicalDefense(0.0f), MagicalDefense(0.0f), MoveSpeed(0.0f)
	{
	}
};