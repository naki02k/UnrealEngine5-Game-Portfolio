#include "ScreenFadeWidget.h"
#include "Animation/WidgetAnimation.h"

void UScreenFadeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 現在のモードに応じて対応するフェードアニメーションを自動再生する
	if (CurrentMode == EFadeMode::FadeOut && FadeOutAnim)
	{
		PlayAnimation(FadeOutAnim);
	}
	else if (CurrentMode == EFadeMode::FadeIn && FadeInAnim)
	{
		PlayAnimation(FadeInAnim);
	}
}
