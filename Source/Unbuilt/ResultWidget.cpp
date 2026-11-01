#include "ResultWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UResultWidget::SetupResult(int32 FinalScore, int32 RankIndex)
{
	TargetScore = FinalScore;
	DisplayScore = 0.0f;
	CurrentRank = RankIndex;
	bIsAnimating = true;
	bStartStarAnim = false;

	if (ScoreText)
	{
		ScoreText->SetText(FText::AsNumber(0));
	}

	// ランクに応じて星のテクスチャを割り当てる
	UTexture2D* LeftTex = (CurrentRank >= 1) ? Texture_FilledStar : Texture_EmptyStar;
	UTexture2D* RightTex = (CurrentRank >= 2) ? Texture_FilledStar : Texture_EmptyStar;
	UTexture2D* CenterTex = (CurrentRank >= 3) ? Texture_FilledStar : Texture_EmptyStar;

	if (StarImage_Left && LeftTex)     StarImage_Left->SetBrushFromTexture(LeftTex);
	if (StarImage_Right && RightTex)   StarImage_Right->SetBrushFromTexture(RightTex);
	if (StarImage_Center && CenterTex) StarImage_Center->SetBrushFromTexture(CenterTex);

	// 初期状態：完全に非表示にしておく
	if (StarImage_Left)   StarImage_Left->SetVisibility(ESlateVisibility::Hidden);
	if (StarImage_Right)  StarImage_Right->SetVisibility(ESlateVisibility::Hidden);
	if (StarImage_Center) StarImage_Center->SetVisibility(ESlateVisibility::Hidden);

	// 一定時間はクリックで戻れないように制限
	bCanClickToReturn = false;
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UResultWidget::EnableClick, 2.0f, false);
}

void UResultWidget::EnableClick()
{
	bCanClickToReturn = true;
}

void UResultWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsAnimating)
	{
		// スコアを滑らかにカウントアップさせる
		DisplayScore = FMath::FInterpTo(DisplayScore, (float)TargetScore, InDeltaTime, 5.0f);

		if (ScoreText)
		{
			ScoreText->SetText(FText::AsNumber(FMath::RoundToInt(DisplayScore)));
		}

		// 目標スコアの80%に到達した時点で星のアニメーションシーケンスを一度だけ開始
		if (DisplayScore > TargetScore * 0.8f && !bStartStarAnim)
		{
			bStartStarAnim = true;
			StartStarSequence();
		}

		// カウントアップか目標値にほぼ到達したら官僚とする
		if (FMath::IsNearlyEqual(DisplayScore, (float)TargetScore, 0.1f))
		{
			DisplayScore = (float)TargetScore;
			if (ScoreText) ScoreText->SetText(FText::AsNumber(TargetScore));
			bIsAnimating = false;
		}
	}
}

// 星のスタンプ演出シーケンスを一括制御
void UResultWidget::StartStarSequence()
{
	// 即座に左の星を再生
	PlayLeftStarAnim();

	// 0.3秒後に右の星を再生
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_RightStar, this, &UResultWidget::PlayRightStarAnim, 0.3f, false);

	// 0.6秒後に真ん中の星を再生
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_CenterStar, this, &UResultWidget::PlayCenterStarAnim, 0.6f, false);
}

void UResultWidget::PlayLeftStarAnim()
{
	if (StarImage_Left)
	{
		StarImage_Left->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (Anim_StarLeft)
		{
			PlayAnimation(Anim_StarLeft);
		}
	}
}

void UResultWidget::PlayRightStarAnim()
{
	if (StarImage_Right)
	{
		StarImage_Right->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (Anim_StarRight)
		{
			PlayAnimation(Anim_StarRight);
		}
	}
}

void UResultWidget::PlayCenterStarAnim()
{
	if (StarImage_Center)
	{
		StarImage_Center->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (Anim_StarCenter)
		{
			PlayAnimation(Anim_StarCenter);
		}
	}
}

FReply UResultWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// クリック許可フラグがたっている場合タイトル画面へ戻る
	if (bCanClickToReturn)
	{
		const FString TitlePath = TEXT("/Game/Maps/Title_L");

		UE_LOG(LogTemp, Log, TEXT("ResultWidget: Transitioning back to Title [%s]"), *TitlePath);

		if (UWorld* World = GetWorld())
		{
			UGameplayStatics::OpenLevel(World, FName(*TitlePath));
		}

		return FReply::Handled();
	}
	return FReply::Unhandled();
}