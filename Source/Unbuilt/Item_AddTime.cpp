#include "Item_AddTime.h"
#include "ScoreManager.h"
#include "Kismet/GameplayStatics.h"

AItem_AddTime::AItem_AddTime()
{
	// アイテムの名前と説明文の初期化
	ItemName = FText::FromString(TEXT("Time Extension"));
	ItemDescription = FText::FromString(TEXT("Adds extra time to the countdown."));
}

void AItem_AddTime::ApplyEffect_Implementation(AActor* TargetActor)
{
	Super::ApplyEffect_Implementation(TargetActor);

	// ワールド内のScoreManagerを検索して取得
	AScoreManager* ScoreManager = Cast<AScoreManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AScoreManager::StaticClass()));

	if (ScoreManager)
	{
		// 制限時間を加算
		ScoreManager->AddTimeLimit(TimeToAdd);
	}
}

