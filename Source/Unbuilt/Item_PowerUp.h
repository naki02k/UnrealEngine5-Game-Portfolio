// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Item_PowerUp.generated.h"

/**
 * 取得時にプレイヤーの攻撃力を一定時間強化するアイテムクラス
 */
UCLASS()
class UNBUILT_API AItem_PowerUp : public AItemBase
{
	GENERATED_BODY()
	
public:
	AItem_PowerUp();

protected:
	// 攻撃力の倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|PowerUp")
	float DamageMultiplier = 2.0f;

	// 効果が持続する時間（秒数）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|PowerUp")
	float Duration = 10.0f;

	// アイテム取得時の効果適用処理
	virtual void ApplyEffect_Implementation(AActor* TargetActor)override;
};
