#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "UnbuiltCharacter.h"
#include "MainHUDWidget.generated.h"

/**
 * ゲームプレイ中のメインHUD（体力、残弾、スコア、カウントダウン等）を制御するウィジェットクラス
 */
UCLASS()
class UNBUILT_API UMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ゲーム開始時のカウントダウンを開始する
	void StartCountdown();

	// HUDの表示情報（制限時間、スコア、手配度など）を更新する
	void UpdateHUD(float RemainingTime, int32 CurrentScore, int32 WantedLevel = 0, float WantedPointPercent = 0.0f);

	// オーナーであるプレイヤーキャラクターを取得する関数
	UFUNCTION(BlueprintPure, Category = "HUD")
	AUnbuiltCharacter* GetOwningCharacter() const {
		return Cast<AUnbuiltCharacter>(GetOwningPlayerPawn());
	}

	// ---ブル―プリントバインド用関数---
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetPlayerHPPercent() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetPlayerXPPercent() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetPlayerLevelText() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetAmmoText() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetReloadBarPercent() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	ESlateVisibility GetReloadBarVisibility() const;

	UFUNCTION(BlueprintPure, Category = "HUD|Wanted")
	FText GetWantedLevelText() const;

	UFUNCTION(BlueprintPure, Category = "HUD|Wanted")
	float GetWantedPointPercent() const;

protected:
	// ---UMGウィジェットバインド---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CountdownText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* WantedLevelText;

	// ---リロード中の点滅アニメーション---
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* Anim_ReloadingBlink;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	// カウントダウンを進めるタイマー用コールバック関数
	void UpdateCountdown();
	int32 CountdownValue = 3;
	FTimerHandle CountdownTimerHandle;

	// 前フレームのリロード状態
	bool bWasReloadingLastFrame = false;

	// キャッシュされた手配度関数データ
	int32 CachedWantedLevel = 0;
	float CachedWantedPointPercent = 0.0f;
};