// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleWidget.h"
#include "ScreenFadeWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"

void UTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 各ボタンのクリックイベントをバインド
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UTitleWidget::OnStartClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UTitleWidget::OnQuitClicked);
	}
	if (DescriptionButton)
	{
		DescriptionButton->OnClicked.AddDynamic(this, &UTitleWidget::OnDescriptionClicked);
	}
}

void UTitleWidget::OnStartClicked()
{
	// ボタンクリック音を鳴らす
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}

	// フェードアウトウィジェットが設定されている場合は、フェード演出を挟んでからレベル遷移する
	if (FadeWidgetClass)
	{
		UScreenFadeWidget* FadeUI = CreateWidget<UScreenFadeWidget>(GetWorld(), FadeWidgetClass);
		if (FadeUI)
		{
			FadeUI->CurrentMode = EFadeMode::FadeOut;
			FadeUI->AddToViewport(999);

			// 連打防止のためスタートボタンを無効化
			StartButton->SetIsEnabled(false);

			// フェード演出の秒数（1秒）に合わせてタイマーを設定し、完了後にレベル移動
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UTitleWidget::MoveToMainLevel, 1.0f, false);
			return;
		}
	}

	// フェードウィジェットがない場合は即座にレベル遷移
	MoveToMainLevel();
}

void UTitleWidget::MoveToMainLevel()
{
	FName LevelPath = TEXT("/Game/Maps/Main_L");

	// ログ出力（Developmentビルドで確認用）
	UE_LOG(LogTemp, Log, TEXT("Level transition started to: %s"), *LevelPath.ToString());

	UGameplayStatics::OpenLevel(this, LevelPath);
}

void UTitleWidget::OnQuitClicked()
{
	// ボタンクリック音を鳴らす
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}

	// ゲームを終了
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

void UTitleWidget::OnDescriptionClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Description Button Clicked!"));
	
	// ボタンクリック音を鳴らす
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}

	// 遊び方（説明）ウィジェットが設定されていれば生成してビューポートに追加する
	if (DescriptionWidgetClass)
	{
		UUserWidget* DescriptionUI = CreateWidget<UUserWidget>(GetWorld(), DescriptionWidgetClass);
		if (DescriptionUI)
		{
			// 画面に表示
			DescriptionUI->AddToViewport(999);
		}
	}
}