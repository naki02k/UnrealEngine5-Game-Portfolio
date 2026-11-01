#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Item_SpeedUp.generated.h"

/**
 * 取得時にプレイヤーの移動速度を一定時間アップするアイテムクラス
 */
UCLASS()
class UNBUILT_API AItem_SpeedUp : public AItemBase
{
	GENERATED_BODY()

public:
	AItem_SpeedUp();

protected:
	// 移動速度の倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Buff")
	float SpeedMultiplier = 1.5f;

	// 効果が持続する時間（秒数）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Buff")
	float Duration = 5.0f;

	// アイテム取得時の効果適用処理
	virtual void ApplyEffect_Implementation(AActor* TargetActor)override;
};
