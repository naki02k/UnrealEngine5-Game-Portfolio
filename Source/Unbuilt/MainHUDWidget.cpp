#include "MainHUDWidget.h"
#include "TimerManager.h"
#include "Animation/WidgetAnimation.h"
#include "ScoreManager.h"
#include "Kismet/GameplayStatics.h"

// プレイヤーのHP割合を取得
float UMainHUDWidget::GetPlayerHPPercent() const {
	AUnbuiltCharacter* Char = GetOwningCharacter();
	return Char ? Char->GetHPPercent() : 0.0f;
}

// プレイヤーの経験値割合を取得
float UMainHUDWidget::GetPlayerXPPercent() const {
	AUnbuiltCharacter* Char = GetOwningCharacter();
	return Char ? Char->GetXPPercent() : 0.0f;
}

// プレイヤーのレベル文字列を取得
FText UMainHUDWidget::GetPlayerLevelText() const {
	AUnbuiltCharacter* Char = GetOwningCharacter();
	if (Char) {
		return FText::Format(NSLOCTEXT("UI", "LevelFormat", "Lv. {0}"), FText::AsNumber(Char->GetCurrentLevelInt()));
	}
	return FText::FromString(TEXT("Lv. --"));
}

// 残弾数テキストを取得（現在弾数 / 最大弾数）
FText UMainHUDWidget::GetAmmoText() const {
	AUnbuiltCharacter* Char = GetOwningCharacter();
	if (Char) {
		return FText::Format(FText::FromString("{0}/{1}"),
			FText::AsNumber(Char->CurrentAmmo),
			FText::AsNumber(Char->WeaponMaxAmmo));
	}
	return FText::GetEmpty();
}

// リロード進捗の割合を取得
float UMainHUDWidget::GetReloadBarPercent() const {
	AUnbuiltCharacter* Char = GetOwningCharacter();
	return Char ? Char->GetReloadProgress() : 0.0f;
}

// リロードバーの表示・非表示を切り替え
ESlateVisibility UMainHUDWidget::GetReloadBarVisibility() const {
	AUnbuiltCharacter* Char = GetOwningCharacter();
	return (Char && Char->bIsReloading) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}

// 手配度レベルのテキストを取得
FText UMainHUDWidget::GetWantedLevelText() const {
	return FText::Format(NSLOCTEXT("UI", "WantedFormat", "WANTED Lv.{0}"), FText::AsNumber(CachedWantedLevel));
}

// 手配度の進行度を取得
float UMainHUDWidget::GetWantedPointPercent() const {
	return CachedWantedPointPercent;
}

void UMainHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	AUnbuiltCharacter* Char = GetOwningCharacter();
	if (!Char || !Anim_ReloadingBlink) return;

	bool bIsCurrentlyReloading = Char->bIsReloading;
	
	// リロード状態が変化した瞬間だけアニメーションの再生/停止を切り替える
	if (bIsCurrentlyReloading != bWasReloadingLastFrame)
	{
		if (bIsCurrentlyReloading)
		{
			// ループ再生
			PlayAnimation(Anim_ReloadingBlink, 0.0f, 0);
		}
		else
		{
			StopAnimation(Anim_ReloadingBlink);
		}
	}
	bWasReloadingLastFrame = bIsCurrentlyReloading;
}

void UMainHUDWidget::StartCountdown() {
	CountdownValue = 3;
	if (CountdownText) {
		CountdownText->SetVisibility(ESlateVisibility::Visible);
		CountdownText->SetText(FText::AsNumber(CountdownValue));
	}
	
	// 1秒ごとにUpdateCountdownを呼び出すタイマー設定
	GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &UMainHUDWidget::UpdateCountdown, 1.0f, true);
}

void UMainHUDWidget::UpdateCountdown() {
	CountdownValue--;
	if (!CountdownText) return;
	if (CountdownValue > 0) CountdownText->SetText(FText::AsNumber(CountdownValue));
	else if (CountdownValue == 0) CountdownText->SetText(FText::FromString(TEXT("START!")));
	else {
		// カウントダウン終了：タイマーをクリアしてテキストを非表示にする
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
		CountdownText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMainHUDWidget::UpdateHUD(float RemainingTime, int32 CurrentScore, int32 WantedLevel, float WantedPointPercent)
{
	// 制限時間の更新（切り上げ表示）
	if (TimeText) {
		TimeText->SetText(FText::AsNumber(FMath::CeilToInt(RemainingTime)));
	}

	// 手配度情報のキャッシュと更新
	CachedWantedLevel = WantedLevel;
	CachedWantedPointPercent = FMath::Clamp(WantedPointPercent, 0.0f, 1.0f);

	if (WantedLevelText) {
		WantedLevelText->SetText(GetWantedLevelText());
	}
}