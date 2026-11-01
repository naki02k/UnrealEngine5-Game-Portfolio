#include "LevelupWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "UnbuiltCharacter.h"
#include "TP_WeaponComponent.h"

// プレイヤーキャラクターから武器コンポーネントを取得する関数
UTP_WeaponComponent* GetWeapon(AUnbuiltCharacter* MyChar)
{
	if (!MyChar) return nullptr;
	UTP_WeaponComponent* Weapon = MyChar->FindComponentByClass<UTP_WeaponComponent>();
	if (!Weapon)
	{
		// アタッチされているアクターからも検索を試みる
		TArray<AActor*> AttachedActors;
		MyChar->GetAttachedActors(AttachedActors);
		for (AActor* Actor : AttachedActors)
		{
			if (Actor) {
				Weapon = Actor->FindComponentByClass<UTP_WeaponComponent>();
				if (Weapon) break;
			}
		}
	}
	return Weapon;
}

void ULevelupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// レベルアップ開始音の再生
	if (LevelUpStartSound)
	{
		UGameplayStatics::PlaySound2D(this, LevelUpStartSound);
	}

	// 各ボタンのクリックイベントをバインド
	if (ChoiceButton_1) ChoiceButton_1->OnClicked.AddDynamic(this, &ULevelupWidget::OnChoice1Clicked);
	if (ChoiceButton_2) ChoiceButton_2->OnClicked.AddDynamic(this, &ULevelupWidget::OnChoice2Clicked);
	if (ChoiceButton_3) ChoiceButton_3->OnClicked.AddDynamic(this, &ULevelupWidget::OnChoice3Clicked);

	// ランダムな選択肢をセットアップ
	SetupRandomChoices();

	// 登場アニメーションの再生
	if (LevelUpAnim) PlayAnimation(LevelUpAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
}

void ULevelupWidget::SetupRandomChoices()
{
	// 選択肢の全候補
	TArray<EUpgradeType> AllTypes = {
		EUpgradeType::FireRate, EUpgradeType::Damage, EUpgradeType::ProjectileCount,
		EUpgradeType::ReloadSpeed, EUpgradeType::MaxAmmo
	};

	// 候補をシャッフル
	int32 LastIndex = AllTypes.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = FMath::RandRange(i, LastIndex);
		if (i != Index) AllTypes.Swap(i, Index);
	}

	UTextBlock* Labels[] = { Label_1, Label_2, Label_3 };
	UTextBlock* Descs[] = { Description_1, Description_2, Description_3 };

	AUnbuiltCharacter* MyChar = Cast<AUnbuiltCharacter>(GetOwningPlayerPawn());
	UTP_WeaponComponent* Weapon = GetWeapon(MyChar);

	// 上位3つを今回の選択肢としてテキストや数値を決定
	for (int32 i = 0; i < 3; i++)
	{
		ButtonChoices[i] = AllTypes[i];
		FString TitleString = TEXT("");
		FString ValueString = TEXT("");

		switch (AllTypes[i])
		{
		case EUpgradeType::FireRate:
			TitleString = TEXT("FIRE RATE UP");
			if (Weapon) ValueString = FString::Printf(TEXT("%.2fs -> %.2fs"), Weapon->FireRate, FMath::Max(Weapon->MinFireInterval, Weapon->FireRate * 0.85f));
			break;

		case EUpgradeType::Damage:
			TitleString = TEXT("DAMAGE UP");
			if (Weapon) ValueString = FString::Printf(TEXT("%.0f -> %.0f"), Weapon->BaseDamage, Weapon->BaseDamage + 50.0f);
			break;

		case EUpgradeType::ProjectileCount:
			TitleString = TEXT("MULTI SHOT");
			if (Weapon) ValueString = FString::Printf(TEXT("%d -> %d Bullets"), Weapon->ProjectileCount, Weapon->ProjectileCount + 1);
			break;

		case EUpgradeType::ReloadSpeed:
			TitleString = TEXT("RELOAD SPEED UP");
			if (MyChar) ValueString = FString::Printf(TEXT("%.2fs -> %.2fs"), MyChar->WeaponReloadTime, FMath::Max(0.3f, MyChar->WeaponReloadTime * 0.85f));
			break;

		case EUpgradeType::MaxAmmo:
			TitleString = TEXT("CAPACITY UP");
			if (MyChar)
			{
				ValueString = FString::Printf(
					TEXT("%d -> %d Bullets"),
					MyChar->WeaponMaxAmmo,
					MyChar->WeaponMaxAmmo + 10
				);
			}
			break;
		}

		// タイトル設定
		if (Labels[i])
		{
			Labels[i]->SetText(FText::FromString(TitleString));
			Labels[i]->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.2f, 1.0f))); // 金色/黄色
		}

		// 詳細説明設定（英語のみ表記で文字化け回避）
		if (Descs[i])
		{
			Descs[i]->SetText(FText::FromString(ValueString));
			Descs[i]->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.0f, 0.4f, 1.0f))); // 明るい緑
		}
	}
}

void ULevelupWidget::OnChoice1Clicked() { ApplyUpgrade(ButtonChoices[0]); }
void ULevelupWidget::OnChoice2Clicked() { ApplyUpgrade(ButtonChoices[1]); }
void ULevelupWidget::OnChoice3Clicked() { ApplyUpgrade(ButtonChoices[2]); }

void ULevelupWidget::ApplyUpgrade(EUpgradeType Type)
{
	// 既に選択済みの場合は処理しない
	if (bHasSelected) return;
	bHasSelected = true;

	// ボタンクリック音を鳴らす
	if (ButtonClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ButtonClickSound);
	}

	AUnbuiltCharacter* MyChar = Cast<AUnbuiltCharacter>(GetOwningPlayerPawn());
	UTP_WeaponComponent* Weapon = GetWeapon(MyChar);

	// 選択された種類に応じてステータスを強化
	switch (Type)
	{
	case EUpgradeType::FireRate:
		if (Weapon) Weapon->FireRate = FMath::Max(Weapon->MinFireInterval, Weapon->FireRate * 0.85f);
		break;
	case EUpgradeType::Damage:
		if (Weapon) Weapon->BaseDamage += 50.0f;
		break;
	case EUpgradeType::ProjectileCount:
		if (Weapon) { Weapon->ProjectileCount += 1; Weapon->SpreadAngle += 2.5f; }
		break;
	case EUpgradeType::ReloadSpeed:
		if (MyChar) MyChar->WeaponReloadTime = FMath::Max(0.3f, MyChar->WeaponReloadTime * 0.85f);
		break;
	case EUpgradeType::MaxAmmo:
		if (MyChar) { MyChar->WeaponMaxAmmo += 10; MyChar->CurrentAmmo = MyChar->WeaponMaxAmmo; }
		break;
	}

	// ゲームを再開
	ResumeGame();
}

void ULevelupWidget::ResumeGame()
{
	bHasSelected = false;
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		// マウスカーソルを非表示にし、入力モードをゲーム専用に戻す
		PC->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		// キャラクター側のレベルアップフラグを下ろす
		if (AUnbuiltCharacter* MyChar = Cast<AUnbuiltCharacter>(PC->GetPawn()))
		{
			MyChar->bIsLevelingUp = false;
		}
	}
	// ウィジェットを画面から削除
	RemoveFromParent();
}