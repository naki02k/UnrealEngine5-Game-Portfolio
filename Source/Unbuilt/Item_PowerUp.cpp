#include "Item_PowerUp.h"
#include "UnbuiltCharacter.h"

AItem_PowerUp::AItem_PowerUp()
{
	// アイテムの名前と説明文の初期化
	ItemName = FText::FromString(TEXT("Attack Power Up"));
	ItemDescription = FText::FromString(TEXT("Increases attack damage for a limited time."));
}

void AItem_PowerUp::ApplyEffect_Implementation(AActor* TargetActor)
{
	Super::ApplyEffect_Implementation(TargetActor);

	// 拾ったアクターが AUnbuiltCharacter か判定
	if (AUnbuiltCharacter* PlayerCharacter = Cast<AUnbuiltCharacter>(TargetActor))
	{
		// 攻撃力アップバフを実行
		PlayerCharacter->PowerUpAttack(DamageMultiplier, Duration);
	}
}