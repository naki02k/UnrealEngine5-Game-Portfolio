// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TitleGameMode.generated.h"

/**
 * タイトル画面専用のゲームモードクラス
 */
UCLASS()
class UNBUILT_API ATitleGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ATitleGameMode();

protected:
	virtual void BeginPlay() override;

	// タイトル画面で使用するウィジェットクラス
	UPROPERTY(EditDefaultsOnly, Category = "Title")
	TSubclassOf<class UUserWidget> TitleWidgetClass;

	// タイトル画面で映し出す専用のカメラアクター
	UPROPERTY(EditAnywhere, Category = "Title")
	class ACameraActor* TitleCameraActor;

};
