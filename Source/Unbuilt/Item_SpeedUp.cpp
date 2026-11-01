// Fill out your copyright notice in the Description page of Project Settings.


#include "Item_SpeedUp.h"
#include "UnbuiltCharacter.h"

AItem_SpeedUp::AItem_SpeedUp()
{
	// アイテムの名前と説明文の初期化
	ItemName = FText::FromString(TEXT("Speed Boost"));
	ItemDescription = FText::FromString(TEXT("Increases movement speed for a limited time."));
}

void AItem_SpeedUp::ApplyEffect_Implementation(AActor* TargetActor)
{
	Super::ApplyEffect_Implementation(TargetActor);

	// 拾ったアクターが AUnbuiltCharacter か判定
	if (AUnbuiltCharacter* PlayerCharacter = Cast<AUnbuiltCharacter>(TargetActor))
	{
		// プレイヤーに移動速度バフを適用
		PlayerCharacter->ApplySpeedBuff(SpeedMultiplier, Duration);
	}
}
