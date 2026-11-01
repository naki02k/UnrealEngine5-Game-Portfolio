// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnbuiltCharacter.h"
#include "ScoreManager.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AUnbuiltCharacter::AUnbuiltCharacter()
{
	// カプセルコンポーネントのサイズを設定
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// ファーストパーソンカメラを作成
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// 1人称視点用メッシュを作成
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// ステータス初期値
	CurrentXP = 0.0f;
	MaxXP = 100.0f;
	CurrentLevel = 1;
}

void AUnbuiltCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAnyFlags(RF_ClassDefaultObject) || !GetWorld())
	{
		return;
	}

	if (WeaponDataTable)
	{
		if (WeaponDataTable->IsValidLowLevel() && !ActiveWeaponRowName.IsNone())
		{
			FGunData* FoundData = WeaponDataTable->FindRow<FGunData>(ActiveWeaponRowName, TEXT("WeaponInitContext"));
			if (FoundData)
			{
				CurrentWeaponData = *FoundData;
				UE_LOG(LogTemplateCharacter, Log, TEXT("Successfully loaded weapon data: %s"), *CurrentWeaponData.GunName);
			}
		}
	}
	else
	{
		CurrentWeaponData = FGunData();
	}

	WeaponMaxAmmo = CurrentWeaponData.MaxAmmo;
	WeaponReloadTime = CurrentWeaponData.ReloadTime;

	// 初期ステータスを設定
	CurrentAmmo = WeaponMaxAmmo;
	CurrentHP = MaxHP;

	// 基準となる移動速度を保存
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		DefaultMaxSpeed = MovementComponent->MaxWalkSpeed;
	}
}

float AUnbuiltCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	ApplyDamage(DamageToApply);

	return DamageToApply;
}

void AUnbuiltCharacter::ApplyDamage(float DamageAmount)
{
	// HPを減少させ、範囲内に収める
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);

	// HPの変化に応じて移動速度を更新
	UpdateStatsEffects();

	// ダメージ時のカメラシェイク
	if (HurtCameraShakeClass)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (PlayerController->PlayerCameraManager)
			{
				PlayerController->PlayerCameraManager->StartCameraShake(HurtCameraShakeClass, 1.0f);
			}
		}
	}

	// HPが0になった場合
	if (CurrentHP <= 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			AActor* ManagerActor = UGameplayStatics::GetActorOfClass(World, AScoreManager::StaticClass());
			if (AScoreManager* ScoreManager = Cast<AScoreManager>(ManagerActor))
			{
				ScoreManager->FinishGame();
			}
		}
	}
}

void AUnbuiltCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Inputによる入力設定
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUnbuiltCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUnbuiltCharacter::Look);
	}

	// ESCキーでポーズメニューを開く
	PlayerInputComponent->BindKey(EKeys::Escape, EInputEvent::IE_Pressed, this, &AUnbuiltCharacter::TogglePauseMenu);
}

void AUnbuiltCharacter::Move(const FInputActionValue& Value)
{
	// レベルアップ中は移動を停止
	if (bIsLevelingUp)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AUnbuiltCharacter::Look(const FInputActionValue& Value)
{
	// レベルアップ中は視点操作を停止
	if (bIsLevelingUp)
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUnbuiltCharacter::AddExperience(float Amount)
{
	// レベルアップ選択中は経験値を加算しない
	if (bIsLevelingUp)
	{
		return;
	}

	CurrentXP += Amount;

	// 必要経験値に達した場合
	if (CurrentXP >= MaxXP)
	{
		bIsLevelingUp = true;

		CurrentXP -= MaxXP;
		MaxXP *= 1.2f;
		CurrentLevel++;

		OnLevelUp();
	}
}

void AUnbuiltCharacter::OnLevelUp()
{
	// レベルアップ時にHPを20%回復
	CurrentHP = FMath::Min(CurrentHP + MaxHP * 0.2f, MaxHP);
	UpdateStatsEffects();

	// 制限時間を追加
	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AScoreManager::StaticClass());

	if (AScoreManager* ScoreManager = Cast<AScoreManager>(ManagerActor))
	{
		ScoreManager->TimeLimit += 5.0f;
	}

	// レベルアップUIを表示
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (LevelUpWidgetClass)
		{
			if (UUserWidget* LevelUpWidget = CreateWidget<UUserWidget>(PlayerController, LevelUpWidgetClass))
			{
				LevelUpWidget->AddToViewport(999);

				PlayerController->bShowMouseCursor = true;

				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(LevelUpWidget->TakeWidget());

				PlayerController->SetInputMode(InputMode);
			}
		}
	}
}

void AUnbuiltCharacter::UpdateStatsEffects()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// HPの割合を計算
		const float HPRatio = (MaxHP > 0.0f) ? CurrentHP / MaxHP : 0.0f;

		// HPが減るほど移動速度を低下させる
		const float BaseSpeed = FMath::Lerp(
			DefaultMaxSpeed * 0.3f,
			DefaultMaxSpeed,
			HPRatio
		);

		MovementComponent->MaxWalkSpeed = BaseSpeed * CurrentSpeedMultiplier;
	}
}

void AUnbuiltCharacter::StartReload()
{
	// リロード中、または弾薬が満タンの場合は処理しない
	if (bIsReloading || CurrentAmmo >= WeaponMaxAmmo)
	{
		return;
	}

	bIsReloading = true;

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&AUnbuiltCharacter::FinishReload,
		WeaponReloadTime,
		false
	);
}

void AUnbuiltCharacter::FinishReload()
{
	bIsReloading = false;
	CurrentAmmo = WeaponMaxAmmo;
}

float AUnbuiltCharacter::GetReloadProgress() const
{
	if (!bIsReloading)
	{
		return 0.0f;
	}

	const float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(ReloadTimerHandle);

	return FMath::Clamp(
		1.0f - RemainingTime / WeaponReloadTime,
		0.0f,
		1.0f
	);
}

void AUnbuiltCharacter::ApplySpeedBuff(float SpeedMultiplier, float Duration)
{
	CurrentSpeedMultiplier = SpeedMultiplier;
	bIsSpeedBuffActive = true;

	UpdateStatsEffects();

	GetWorld()->GetTimerManager().SetTimer(
		SpeedBuffTimerHandle,
		this,
		&AUnbuiltCharacter::ResetSpeedBuff,
		Duration,
		false
	);
}

void AUnbuiltCharacter::ResetSpeedBuff()
{
	CurrentSpeedMultiplier = 1.0f;
	bIsSpeedBuffActive = false;

	UpdateStatsEffects();
}

void AUnbuiltCharacter::PowerUpAttack(float Multiplier, float Duration)
{
	DamageMultiplier = Multiplier;

	GetWorld()->GetTimerManager().SetTimer(
		PowerUpTimerHandle,
		this,
		&AUnbuiltCharacter::ResetAttackDamage,
		Duration,
		false
	);
}

void AUnbuiltCharacter::ResetAttackDamage()
{
	DamageMultiplier = 1.0f;
}

void AUnbuiltCharacter::TogglePauseMenu()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!PlayerController || !PauseMenuWidgetClass)
	{
		return;
	}

	UUserWidget* PauseWidget = CreateWidget<UUserWidget>(
		PlayerController,
		PauseMenuWidgetClass
	);

	if (!PauseWidget)
	{
		return;
	}

	PauseWidget->AddToViewport(999);

	// マウスカーソルを表示してUI操作に切り替える
	PlayerController->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseWidget->TakeWidget());

	PlayerController->SetInputMode(InputMode);

	// ゲームを一時停止
	UGameplayStatics::SetGamePaused(this, true);
}