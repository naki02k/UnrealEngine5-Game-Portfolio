#include "TitleGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

ATitleGameMode::ATitleGameMode()
{
	// タイトルではプレイヤーアクターを生成しない
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}

void ATitleGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		// レベル内のカメラを自動検索してセット
		AActor* FoundCamera = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass());
		if (FoundCamera)
		{
			TitleCameraActor = Cast<ACameraActor>(FoundCamera);
			PC->SetViewTargetWithBlend(TitleCameraActor);
		}

		// UIの生成と表示
		if (TitleWidgetClass)
		{
			UUserWidget* TitleUI = CreateWidget<UUserWidget>(GetWorld(), TitleWidgetClass);
			if (TitleUI) TitleUI->AddToViewport();
		}

		// マウス設定
		PC->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
	}
}