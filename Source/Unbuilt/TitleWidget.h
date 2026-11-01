#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleWidget.generated.h"

class USoundBase;
/**
 * タイトル画面のメニュー操作（ゲーム開始、終了、遊び方表示、フェード遷移）を管理するウィジェットクラス
 */
UCLASS()
class UNBUILT_API UTitleWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// ---UMGウィジェットバインド---
	UPROPERTY(meta = (BindWidget))
	class UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* DescriptionButton;

	// ---ウィジェットクラス指定---
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> FadeWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> DescriptionWidgetClass;

	// ---サウンド---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> ClickSound;

	// メインレベルへ実際にレベル遷移する関数
	void MoveToMainLevel();

	virtual void NativeConstruct() override;

	// ---ボタンクリック時のコールバック関数---
	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnDescriptionClicked();
};
