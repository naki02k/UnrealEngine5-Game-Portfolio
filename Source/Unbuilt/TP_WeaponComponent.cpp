#include "TP_WeaponComponent.h"
#include "UnbuiltCharacter.h"
#include "UnbuiltProjectile.h"
#include "ScoreManager.h"

#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);

	// デフォルト値を設定
	FireRate = 0.5f;
	ProjectileCount = 1;
	SpreadAngle = 0.0f;
	bCanFire = true;
}

void UTP_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ゲームワールドからスコアマネージャーをキャッシュしておく
	if (UWorld* World = GetWorld())
	{
		CachedScoreManager = Cast<AScoreManager>(
			UGameplayStatics::GetActorOfClass(
				World,
				AScoreManager::StaticClass()
			)
		);
	}

	// ScoreManagerが取得できなかった場合はログを出力
	if (!IsValid(CachedScoreManager))
	{
		UE_LOG(LogTemp, Error, TEXT("ScoreManager could not be found."));
	}
}

void UTP_WeaponComponent::Fire()
{
	// ScoreManagerが取得できない場合、またはゲームオーバー時は発射不可
	if (!IsValid(CachedScoreManager) || CachedScoreManager->bIsGameOver)
	{
		return;
	}

	// 発射条件のチェック
	if (!IsValid(Character) ||
		Character->bIsReloading ||
		Character->CurrentAmmo <= 0 ||
		!bCanFire)
	{
		// 弾が切れていて、かつリロード中でない場合はリロードを開始
		if (IsValid(Character) &&
			Character->CurrentAmmo <= 0 &&
			!Character->bIsReloading)
		{
			Character->StartReload();
		}

		return;
	}

	UWorld* const World = GetWorld();

	if (!World || !IsValid(ProjectileClass))
	{
		return;
	}

	// PlayerControllerを取得
	APlayerController* PlayerController =
		Cast<APlayerController>(Character->GetController());

	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		return;
	}

	// カメラの向きを取得
	const FRotator SpawnRotation =
		PlayerController->PlayerCameraManager->GetCameraRotation();

	// 弾の生成位置を計算
	const FVector SpawnLocation =
		GetComponentLocation() + SpawnRotation.RotateVector(MuzzleOffset);

	FActorSpawnParameters ActorSpawnParams;

	ActorSpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	// [修正] CurrentWeaponData からの上書きを削除。
	// レベルアップで強化されたメンバー変数の値をそのまま使う
	const int32 ShotCount = FMath::Max(1, ProjectileCount);
	const float ShotSpread = FMath::Max(0.0f, SpreadAngle);

	// 指定された弾数分ループ
	bool bSpawnedProjectile = false;

	for (int32 i = 0; i < ShotCount; i++)
	{
		FRotator FinalRotation = SpawnRotation;

		// 拡散角度の計算
		if (ShotSpread > 0.0f)
		{
			FinalRotation.Pitch += FMath::FRandRange(-ShotSpread, ShotSpread);
			FinalRotation.Yaw += FMath::FRandRange(-ShotSpread, ShotSpread);
		}

		// 弾を生成
		AUnbuiltProjectile* Projectile =
			World->SpawnActor<AUnbuiltProjectile>(
				ProjectileClass,
				SpawnLocation,
				FinalRotation,
				ActorSpawnParams
			);

		// 弾の生成に成功した場合
		if (IsValid(Projectile))
		{
			// ダメージ倍率を反映
			Projectile->AttackDamage =
				BaseDamage * Character->DamageMultiplier;

			bSpawnedProjectile = true;
		}
	}

	// すべての弾の生成に失敗した場合は、弾数を消費しない
	if (!bSpawnedProjectile)
	{
		return;
	}

	// 連射制御
	bCanFire = false;

	// [修正] メンバーの FireRate を使い、下限を MinFireInterval で制限
	const float SafeFireRate = FMath::Max(MinFireInterval, FireRate);

	World->GetTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&UTP_WeaponComponent::ResetFire,
		SafeFireRate,
		false
	);

	// 弾数を減らす
	Character->CurrentAmmo--;

	// 発射音の再生
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FireSound,
			Character->GetActorLocation()
		);
	}

	// 発射アニメーションの再生
	if (FireAnimation != nullptr)
	{
		if (USkeletalMeshComponent* CharacterMesh = Character->GetMesh1P())
		{
			if (UAnimInstance* AnimInstance =
				CharacterMesh->GetAnimInstance())
			{
				AnimInstance->Montage_Play(FireAnimation, 1.0f);
			}
		}
	}
}

void UTP_WeaponComponent::ResetFire()
{
	bCanFire = true;
}

bool UTP_WeaponComponent::AttachWeapon(AUnbuiltCharacter* TargetCharacter)
{
	// Characterの有効性を確認
	if (!IsValid(TargetCharacter))
	{
		return false;
	}

	// すでに武器コンポーネントが登録されている場合は処理しない
	if (TargetCharacter->GetInstanceComponents()
		.FindItemByClass<UTP_WeaponComponent>())
	{
		return false;
	}

	// Worldの有効性を確認
	UWorld* World = GetWorld();

	if (!World || World->bIsTearingDown)
	{
		return false;
	}

	// キャラクターのメッシュを取得
	USkeletalMeshComponent* CharacterMesh =
		TargetCharacter->GetMesh1P();

	if (!IsValid(CharacterMesh))
	{
		return false;
	}

	// Characterを設定
	Character = TargetCharacter;

	// [修正] 武器データの初期値はここで「一度だけ」反映する
	// FireRate が 0 以下(未設定)ならコンストラクタのデフォルト値を維持
	if (Character->CurrentWeaponData.FireRate > 0.0f)
	{
		FireRate = Character->CurrentWeaponData.FireRate;
	}
	FireRate = FMath::Max(MinFireInterval, FireRate);

	ProjectileCount =
		FMath::Max(1, Character->CurrentWeaponData.ProjectileCount);

	SpreadAngle =
		FMath::Max(0.0f, Character->CurrentWeaponData.SpreadAngle);

	UE_LOG(LogTemp, Log, TEXT("Weapon attached: FireRate=%.2f Count=%d Spread=%.1f"),
		FireRate, ProjectileCount, SpreadAngle);

	// 初期弾数を設定
	Character->CurrentAmmo = Character->WeaponMaxAmmo;

	// 武器をキャラクターにアタッチ
	FAttachmentTransformRules AttachmentRules(
		EAttachmentRule::SnapToTarget,
		true
	);

	if (!AttachToComponent(
		CharacterMesh,
		AttachmentRules,
		FName(TEXT("GripPoint"))
	))
	{
		Character = nullptr;
		return false;
	}

	// キャラクターのInstanceComponentに登録
	Character->AddInstanceComponent(this);

	// 入力設定
	if (APlayerController* PlayerController =
		Cast<APlayerController>(Character->GetController()))
	{
		// Enhanced Input Subsystemを取得
		if (ULocalPlayer* LocalPlayer =
			PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
					LocalPlayer
				))
			{
				Subsystem->AddMappingContext(
					FireMappingContext,
					1
				);
			}
		}

		// Enhanced Input Componentを取得
		if (UEnhancedInputComponent* EnhancedInputComponent =
			Cast<UEnhancedInputComponent>(
				PlayerController->InputComponent
			))
		{
			// [修正] 連射モードに応じてトリガーを切り替え
			// Triggered: 押している間ずっと呼ばれる(フルオート)
			// Started  : 押した瞬間に1回だけ呼ばれる(セミオート)
			const ETriggerEvent FireTrigger =
				bIsAutomatic ? ETriggerEvent::Triggered : ETriggerEvent::Started;

			EnhancedInputComponent->BindAction(
				FireAction,
				FireTrigger,
				this,
				&UTP_WeaponComponent::Fire
			);
		}
	}

	return true;
}

void UTP_WeaponComponent::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	// 発射タイマーを解除
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}

	// MappingContextを解除
	if (IsValid(Character))
	{
		if (APlayerController* PlayerController =
			Cast<APlayerController>(Character->GetController()))
		{
			if (ULocalPlayer* LocalPlayer =
				PlayerController->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
					ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
						LocalPlayer
					))
				{
					Subsystem->RemoveMappingContext(
						FireMappingContext
					);
				}
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}