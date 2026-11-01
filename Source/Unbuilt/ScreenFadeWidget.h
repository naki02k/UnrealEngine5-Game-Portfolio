// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScreenFadeWidget.generated.h"

// フェードモードの種類
UENUM(BlueprintType)
enum class EFadeMode : uint8
{
	FadeOut,
	FadeIn
};

/**
 * 画面のフェードイン・フェードアウト演出を管理するウィジェットクラス
 */
UCLASS()
class UNBUILT_API UScreenFadeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// 現在のフェードモード
	UPROPERTY(BlueprintReadWrite, Category = "Fade")
	EFadeMode CurrentMode = EFadeMode::FadeOut;

protected:
	// フェードアウト用のアニメーション
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* FadeOutAnim;

	// フェードイン用のアニメーション
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* FadeInAnim;

	virtual void NativeConstruct()override;
};
