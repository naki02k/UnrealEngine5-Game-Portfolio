#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture2D.h"
#include "MainHUDWidget.h"
#include "ResultWidget.h"
#include "ScoreManager.generated.h"

class AWarnExplosionActor;
class AUnbuiltCharacter;

/**
 * ゲーム全体のスコア、制限時間、手配度（Wanted）、リザルトへの移行を管理するマネージャークラス
 */
UCLASS()
class UNBUILT_API AScoreManager : public AActor
{
	GENERATED_BODY()

public:
	AScoreManager();

protected:
	virtual void BeginPlay() override;

	// リザルト画面ウィジェットクラス
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UResultWidget> ResultWidgetClass;

	// スコア評価関連
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score | Thresholds")
	int32 ScoreThresholdS = 750000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score | Thresholds")
	int32 ScoreThresholdA = 500000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score | Thresholds")
	int32 ScoreThresholdB = 250000;

	// ランク用テクスチャ
	UPROPERTY(EditAnywhere, Category = "UI|Ranks")
	UTexture2D* TextureS;

	UPROPERTY(EditAnywhere, Category = "UI|Ranks")
	UTexture2D* TextureA;

	UPROPERTY(EditAnywhere, Category = "UI|Ranks")
	UTexture2D* TextureB;

	UPROPERTY(EditAnywhere, Category = "UI|Ranks")
	UTexture2D* TextureC;

	// 手配度システム
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wanted")
	int32 CurrentWantedLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wanted")
	int32 MaxWantedLevel = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wanted")
	float WantedPoint = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wanted")
	float MaxWantedPoint = 100.0f;

	// スポーン設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wanted|Spawn")
	TSubclassOf<AActor> WantedSpawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wanted|Spawn")
	TSubclassOf<AWarnExplosionActor> WarningExplosionClass;

	// キャッシュされたスポーン地点のトランスフォーム配列
	TArray<FTransform> CachedSpawnTransforms;

	int32 CurrentSpawnIndex = 0;

	// スポーン地点を事前にキャッシュする
	void CacheSpawnPoints();

	// 指定した数の手配度アクターをスポーンする
	void SpawnWantedActors(int32 SpawnCount);

	// プレイヤーの周囲にランダムな警告爆発を生成する
	void SpawnRandomExplosionAroundPlayer();

private:
	UPROPERTY()
	UMainHUDWidget* ActiveHUD;

	// 爆発スポーン用のタイマーハンドル
	FTimerHandle ExplosionSpawnTimerHandle;

	// 開始時に取得したプレイヤーの参照
	UPROPERTY()
	AUnbuiltCharacter* CachedPlayerChar;

	// HUD表示を更新する
	void UpdateHUDDisplay();

public:
	// アクティブなHUDを設定する
	void SetActiveHUD(UMainHUDWidget* InHUD)
	{
		ActiveHUD = InHUD;
	}

	// タイマー計測を開始してよいかのフラグ
	bool bCanStartTimer = false;

	virtual void Tick(float DeltaTime) override;

	void AddScore(int32 Amount);

	// 手配度ポイントを加算する
	UFUNCTION(BlueprintCallable, Category = "Wanted")
	void AddWantedPoint(float Amount);

	// 現在の手配度レベルを取得
	UFUNCTION(BlueprintPure, Category = "Wanted")
	int32 GetCurrentWantedLevel() const
	{
		return CurrentWantedLevel;
	}

	// 手配度の割合を取得
	UFUNCTION(BlueprintPure, Category = "Wanted")
	float GetWantedPointPercent() const
	{
		return (MaxWantedPoint > 0.0f)
			? (WantedPoint / MaxWantedPoint)
			: 0.0f;
	}

	// ゲームルール設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameRule")
	float TimeLimit = 60.0f;

	UPROPERTY(BlueprintReadOnly, Category = "GameRule")
	int32 TotalScore = 0;

	UPROPERTY(BlueprintReadOnly, Category = "GameRule")
	bool bIsGameOver = false;

	// スコアに応じたランクインデックスを取得
	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetRankIndex() const;

	// ゲームオーバーをトリガーする
	void TriggerGameOver();

	// ゲーム終了処理
	void FinishGame();

	// 制限時間を追加するアイテム等の処理用
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddTimeLimit(float Amount);

	// プレイヤーのHPが変化した時に呼び出す
	void OnPlayerHealthChanged(float NewHP);
};