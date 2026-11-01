#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Animation/WidgetAnimation.h"
#include "ResultWidget.generated.h"

/**
 * リザルト画面（スコア表示、星演出、タイトル復帰）を管理するウィジェットクラス
 */
UCLASS()
class UNBUILT_API UResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ---UMGウィジェットバインド---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> StarImage_Left;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> StarImage_Right;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> StarImage_Center;

	// ---ウィジェットアニメーション---
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_StarLeft;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_StarRight;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_StarCenter;

	// ---星のテクスチャ素材---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result | Textures")
	TObjectPtr<UTexture2D> Texture_EmptyStar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result | Textures")
	TObjectPtr<UTexture2D> Texture_FilledStar;

	// ---リザルトデータの初期化と演出のセットアップ---
	void SetupResult(int32 FinalScore, int32 RankIndex);

private:
	int32 TargetScore = 0;
	float DisplayScore = 0.0f;
	int32 CurrentRank = 0;

	bool bIsAnimating = false;
	bool bStartStarAnim = false;

	// タイマーハンドル
	FTimerHandle TimerHandle_RightStar;
	FTimerHandle TimerHandle_CenterStar;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	bool bCanClickToReturn = false;
	// クリック受付を有効する関数
	void EnableClick();

	// 星の登場アニメーション連鎖を管理
	void StartStarSequence();
	void PlayLeftStarAnim();
	void PlayRightStarAnim();
	void PlayCenterStarAnim();
};