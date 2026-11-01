// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Item_AddTime.generated.h"

/**
 * 取得時に制限時間を追加するアイテムクラス
 */
UCLASS()
class UNBUILT_API AItem_AddTime : public AItemBase
{
	GENERATED_BODY()
	
public:
	AItem_AddTime();

protected:
	// 追加する制限時間（秒数）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Time")
	float TimeToAdd = 2.0f;

	// アイテム取得時の買うか適用処理
	virtual void ApplyEffect_Implementation(AActor* TargetActor) override;
};
