// Fill out your copyright notice in the Description page of Project Settings.

#include "ScoreManager.h"
#include "UnbuiltCharacter.h"
#include "TP_WeaponComponent.h"
#include "WarnExplosionActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"

AScoreManager::AScoreManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AScoreManager::BeginPlay()
{
	Super::BeginPlay();

	// ゲーム開始時にプレイヤーを一度だけ取得してキャッシュする
	CachedPlayerChar = Cast<AUnbuiltCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (CachedPlayerChar)
	{
		UE_LOG(LogTemp, Log, TEXT("Player Character cached successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to cache Player Character."));
	}

	// ゲーム開始時に1度だけレベル上のTargetPointを検索してキャッシュする
	CacheSpawnPoints();
}

void AScoreManager::CacheSpawnPoints()
{
	CachedSpawnTransforms.Empty();

	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), FoundPoints);

	for (AActor* Point : FoundPoints)
	{
		if (Point)
		{
			CachedSpawnTransforms.Add(Point->GetActorTransform());
		}
	}

	// 取得したTargetPointの順番をランダムにシャッフルする
	for (int32 i = CachedSpawnTransforms.Num() - 1; i > 0; i--)
	{
		const int32 j = FMath::RandRange(0, i);
		CachedSpawnTransforms.Swap(i, j);
	}

	UE_LOG(LogTemp, Log, TEXT("Cached %d spawn points at GameStart."), CachedSpawnTransforms.Num());
}

void AScoreManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// キャッシュしたプレイヤーを使用する
	AUnbuiltCharacter* MyChar = CachedPlayerChar;

	// HP監視はOnPlayerHealthChanged()で行うため、
	// ここではプレイヤーのHPを毎フレーム確認しない

	bool bShouldCountDown = bCanStartTimer && !bIsGameOver && TimeLimit > 0.0f;

	// レベルアップ選択中はタイマーを進めない
	if (MyChar && MyChar->bIsLevelingUp)
	{
		bShouldCountDown = false;
	}

	if (bShouldCountDown)
	{
		TimeLimit -= DeltaTime;

		if (TimeLimit <= 0.0f)
		{
			TimeLimit = 0.0f;

			// タイムアップでゲーム終了
			FinishGame();
			return;
		}
	}

	// HUDを更新
	UpdateHUDDisplay();
}

void AScoreManager::UpdateHUDDisplay()
{
	if (!ActiveHUD)
	{
		return;
	}

	const float WantedPercent = (MaxWantedPoint > 0.0f) ? (WantedPoint / MaxWantedPoint) : 0.0f;
	ActiveHUD->UpdateHUD(TimeLimit, TotalScore, CurrentWantedLevel, WantedPercent);
}

void AScoreManager::OnPlayerHealthChanged(float NewHP)
{
	if (bIsGameOver)
	{
		return;
	}

	if (NewHP <= 0.0f)
	{
		FinishGame();
	}
}

void AScoreManager::AddScore(int32 Amount)
{
	TotalScore += Amount;
	UpdateHUDDisplay();
}

void AScoreManager::AddWantedPoint(float Amount)
{
	// ゲームオーバー時、上限到達時、タイマー未開始時は加算しない
	if (bIsGameOver || CurrentWantedLevel >= MaxWantedLevel || Amount <= 0.0f || !bCanStartTimer)
	{
		return;
	}

	WantedPoint += Amount;

	// ポイントが最大値を越えたら手配度レベルを上昇させる
	while (WantedPoint >= MaxWantedPoint && CurrentWantedLevel < MaxWantedLevel)
	{
		WantedPoint -= MaxWantedPoint;
		CurrentWantedLevel++;

		// レベルが上がるごとに次の必要ポイントを増やす
		MaxWantedPoint *= 1.2f;

		UE_LOG(LogTemp, Log, TEXT("Wanted Level Up! Current Level: %d"), CurrentWantedLevel);

		// 手配度レベルに応じたイベント・ギミック発動
		if (CurrentWantedLevel == 1)
		{
			SpawnWantedActors(4);
			UE_LOG(LogTemp, Log, TEXT("Wanted Level 1 Reached: Spawned Wanted Actors once!"));
		}
		else if (CurrentWantedLevel == 2)
		{
			const float RandomInterval = FMath::RandRange(3.0f, 5.0f);
			GetWorld()->GetTimerManager().SetTimer(ExplosionSpawnTimerHandle, this, &AScoreManager::SpawnRandomExplosionAroundPlayer, RandomInterval, false);
			UE_LOG(LogTemp, Log, TEXT("Wanted Level 2 Reached: Started random explosion timer!"));
		}
		else if (CurrentWantedLevel == 3)
		{
			SpawnWantedActors(4);
			UE_LOG(LogTemp, Log, TEXT("Wanted Level 3: Spawned another 4 Turrets!"));
		}
		else if (CurrentWantedLevel == 4)
		{
			UE_LOG(LogTemp, Log, TEXT("Wanted Level 4: Explosion radius increased!"));
		}
		else if (CurrentWantedLevel == 5)
		{
			SpawnWantedActors(4);
			UE_LOG(LogTemp, Log, TEXT("Wanted Level 5: Spawned final Turrets at remaining points!"));
		}

		// 最大レベルに達したら端数をクリア
		if (CurrentWantedLevel >= MaxWantedLevel)
		{
			WantedPoint = 0.0f;
			break;
		}
	}

	UpdateHUDDisplay();
}

void AScoreManager::SpawnWantedActors(int32 SpawnCount)
{
	if (!WantedSpawnClass || CachedSpawnTransforms.Num() == 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	int32 SpawnedCount = 0;

	// シャッフルされた配列からまだ使っていないインデックスを順番に使用する
	while (SpawnedCount < SpawnCount && CurrentSpawnIndex < CachedSpawnTransforms.Num())
	{
		const FTransform& SpawnTransform = CachedSpawnTransforms[CurrentSpawnIndex];

		GetWorld()->SpawnActor<AActor>(WantedSpawnClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), SpawnParams);

		CurrentSpawnIndex++;
		SpawnedCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully spawned %d actors. Current index: %d"), SpawnedCount, CurrentSpawnIndex);
}

void AScoreManager::SpawnRandomExplosionAroundPlayer()
{
	if (!WarningExplosionClass)
	{
		return;
	}

	// キャッシュしたプレイヤーを使用する
	AUnbuiltCharacter* PlayerChar = CachedPlayerChar;
	if (!PlayerChar)
	{
		return;
	}

	const FVector PlayerLoc = PlayerChar->GetActorLocation();

	// プレイヤーを中心としたランダムな角度と距離
	const float RandomAngle = FMath::RandRange(0.0f, 360.0f);
	const float RandomDistance = FMath::RandRange(20.0f, 300.0f);

	const FVector Offset(
		FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
		FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
		0.0f
	);

	const FVector SpawnLocation = PlayerLoc + Offset;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 爆発アクターを生成
	AWarnExplosionActor* ExplosionActor = GetWorld()->SpawnActor<AWarnExplosionActor>(WarningExplosionClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	// レベル4以上の場合は爆発範囲を広げる
	if (ExplosionActor && CurrentWantedLevel >= 4)
	{
		ExplosionActor->SetActorScale3D(FVector(1.5f, 1.5f, 1.5f));
		ExplosionActor->ExplosionRadius *= 1.5f;
	}

	// 次回の爆発を3～5秒後に再度セットする
	if (!bIsGameOver)
	{
		const float NextInterval = FMath::RandRange(3.0f, 5.0f);
		GetWorld()->GetTimerManager().SetTimer(ExplosionSpawnTimerHandle, this, &AScoreManager::SpawnRandomExplosionAroundPlayer, NextInterval, false);
	}
}

int32 AScoreManager::GetRankIndex() const
{
	if (TotalScore >= ScoreThresholdS) return 3;
	if (TotalScore >= ScoreThresholdA) return 2;
	if (TotalScore >= ScoreThresholdB) return 1;
	return 0;
}

void AScoreManager::TriggerGameOver()
{
	FinishGame();
}

void AScoreManager::FinishGame()
{
	// すでにゲームオーバー処理が走っているなら二重実行を防ぐ
	if (bIsGameOver)
	{
		return;
	}

	bIsGameOver = true;

	// 爆発スポーン用のタイマーを停止
	GetWorld()->GetTimerManager().ClearTimer(ExplosionSpawnTimerHandle);

	// キャッシュしたプレイヤーを使用
	AUnbuiltCharacter* MyChar = CachedPlayerChar;
	if (MyChar)
	{
		// 武器コンポーネントを取得して連射タイマーをクリアする
		UTP_WeaponComponent* Weapon = MyChar->FindComponentByClass<UTP_WeaponComponent>();
		if (Weapon)
		{
			GetWorld()->GetTimerManager().ClearAllTimersForObject(Weapon);
		}

		// プレイヤーの入力を無効化
		APlayerController* CharacterPC = Cast<APlayerController>(MyChar->GetController());
		MyChar->DisableInput(CharacterPC);
	}

	// プレイヤーコントローラーの設定
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());
	}

	// メインHUDを画面から削除
	if (ActiveHUD)
	{
		ActiveHUD->RemoveFromParent();
	}

	// リザルト画面ウィジェットを生成してビューポートに追加
	if (ResultWidgetClass)
	{
		UResultWidget* ResultUI = CreateWidget<UResultWidget>(GetWorld(), ResultWidgetClass);
		if (ResultUI)
		{
			ResultUI->AddToViewport(999);
			ResultUI->SetupResult(TotalScore, GetRankIndex());
		}
	}
}

void AScoreManager::AddTimeLimit(float Amount)
{
	if (!bIsGameOver)
	{
		TimeLimit += Amount;
		UE_LOG(LogTemp, Log, TEXT("Time Added: %f. New TimeLimit: %f"), Amount, TimeLimit);
		UpdateHUDDisplay();
	}
}