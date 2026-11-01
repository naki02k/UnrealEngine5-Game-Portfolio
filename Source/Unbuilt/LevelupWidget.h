#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelupWidget.generated.h"

// レベルアップ時のアップグレード項目の種類
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	FireRate,
	Damage,
	ProjectileCount,
	ReloadSpeed,
	MaxAmmo
};

class UButton;
class USoundBase;
class UTextBlock;

/**
 * レベルアップ時の3択選択画面を管理するウィジェットクラス
 */
UCLASS()
class UNBUILT_API ULevelupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 各選択肢ボタンがクリックされたときのコールバック関数
	UFUNCTION() void OnChoice1Clicked();
	UFUNCTION() void OnChoice2Clicked();
	UFUNCTION() void OnChoice3Clicked();

protected:
	// ---UMGウィジェットバインド---
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ChoiceButton_1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ChoiceButton_2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ChoiceButton_3;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Label_1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Label_2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Label_3;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Description_1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Description_2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Description_3;

	// ---サウンド設定---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	TObjectPtr<USoundBase> LevelUpStartSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	TObjectPtr<USoundBase> ButtonClickSound;

	// ---アニメーション---
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> LevelUpAnim;

	virtual void NativeConstruct() override;

	// ゲームを再開する
	void ResumeGame();

	// ランダムに３つの選択肢を選んでUIに反映する
	void SetupRandomChoices();

	// 選択されたアップグレード内容をプレイヤーや武器に適用する
	void ApplyUpgrade(EUpgradeType Type);

private:
	// 二重選択防止フラグ
	bool bHasSelected = false;
	// 現在提示されている３つの選択肢
	EUpgradeType ButtonChoices[3];
};