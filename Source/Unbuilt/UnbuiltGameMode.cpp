// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnbuiltGameMode.h"
#include "UnbuiltCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "ScreenFadeWidget.h"
#include "MainHUDWidget.h"
#include "ScoreManager.h"

AUnbuiltGameMode::AUnbuiltGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void AUnbuiltGameMode::BeginPlay()
{
	Super::BeginPlay();

	// プレイヤーコントローラーを取得
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	// フェードインを表示
	if (FadeWidgetClass)
	{
		UScreenFadeWidget* FadeUI = CreateWidget<UScreenFadeWidget>(GetWorld(), FadeWidgetClass);
		if (FadeUI)
		{
			FadeUI->CurrentMode = EFadeMode::FadeIn;
			FadeUI->AddToViewport(999);
		}
	}

	//　メインHUDを生成
	if (MainHUDClass)
	{
		UMainHUDWidget* MainHUD = CreateWidget<UMainHUDWidget>(GetWorld(), MainHUDClass);
		if (MainHUD)
		{
			MainHUD->AddToViewport(100);

			// ScoreManagerを探してHUDをセット
			AScoreManager* ScoreManager = Cast<AScoreManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AScoreManager::StaticClass()));
			if (ScoreManager)
			{
				ScoreManager->SetActiveHUD(MainHUD);
				ScoreManager->bCanStartTimer = false;
			}

			// プレイヤーを停止させる
			if (AUnbuiltCharacter* Char = Cast<AUnbuiltCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				Char->bIsLevelingUp = true;
			}

			// カウントダウンアニメーション
			MainHUD->StartCountdown();

			// ４秒後にスタート
			FTimerHandle StartTimer;
			GetWorld()->GetTimerManager().SetTimer(StartTimer, this, &AUnbuiltGameMode::StartGameLogic, 4.0f, false);
		}
	}
}

void AUnbuiltGameMode::StartGameLogic()
{
	// プレイヤーの制限を解除
	if (AUnbuiltCharacter* Char = Cast<AUnbuiltCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		Char->bIsLevelingUp = false;
	}

	// ScoreManagerのタイマーを開始させる
	AScoreManager* ScoreManager = Cast<AScoreManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AScoreManager::StaticClass()));
	if (ScoreManager)
	{
		ScoreManager->bCanStartTimer = true;
	}
}