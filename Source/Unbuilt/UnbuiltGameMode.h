#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UnbuiltGameMode.generated.h"

/**
 * ゲーム本編のルールや初期化、HUD・フェード画面の生成を管理するゲームモードクラス
 */
UCLASS(minimalapi)
class AUnbuiltGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUnbuiltGameMode();

protected:
	virtual void BeginPlay() override;

	// ---ウィジェットクラス設定---
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UScreenFadeWidget> FadeWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UMainHUDWidget> MainHUDClass;

	// ゲームの開始ロジック制御する関数
	void StartGameLogic();
};