#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Engine/DataTable.h"
#include "UnbuiltCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UCameraShakeBase;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

USTRUCT(BlueprintType)
struct FGunData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FString GunName = TEXT("Default Rifle");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxAmmo = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ReloadTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 ProjectileCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float SpreadAngle = 0.0f;
};

/**
 * プレイヤーキャラクターの移動、ステータス、弾薬、バフ効果などを管理するクラス
 */
UCLASS(config = Game)
class UNBUILT_API AUnbuiltCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AUnbuiltCharacter();

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// HP残量に応じた移動速度などを更新
	void UpdateStatsEffects();

public:
	// ---ダメージ関連---

	// エンジンのダメージシステムからの通知を受け取る
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyDamage(float DamageAmount);

	// ---ステータス関連---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxXP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 CurrentLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	bool bIsLevelingUp = false;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHPPercent() const { return (MaxHP > 0.0f) ? CurrentHP / MaxHP : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetXPPercent() const { return (MaxXP > 0.0f) ? CurrentXP / MaxXP : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetCurrentLevelInt() const { return CurrentLevel; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddExperience(float Amount);

	void OnLevelUp();

	// ---UI関連---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> LevelUpWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> PauseMenuWidgetClass;

	// --- データテーブル関連 ---
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Setup")
	UDataTable* WeaponDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Setup")
	FName ActiveWeaponRowName = TEXT("Default");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Current")
	FGunData CurrentWeaponData;

	// ---弾薬・リロード関連---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 CurrentAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 WeaponMaxAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	bool bIsReloading = false;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Ammo")
	float WeaponReloadTime = 1.5f;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void StartReload();

	UFUNCTION(BlueprintPure, Category = "Ammo")
	float GetReloadProgress() const;

	// ---バフ効果関連---
	UFUNCTION(BlueprintCallable, Category = "Stats|Buff")
	void ApplySpeedBuff(float SpeedMultiplier, float Duration);

	UFUNCTION(BlueprintPure, Category = "Stats|Buff")
	bool IsSpeedBuffActive() const { return bIsSpeedBuffActive; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseAttackDamage = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float DamageMultiplier = 1.0f;

	void PowerUpAttack(float Multiplier, float Duration);

	// ---ポーズメニュー---
	void TogglePauseMenu();

	// ---エフェクト---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	TSubclassOf<UCameraShakeBase> HurtCameraShakeClass;

	// ---コンポーネント取得---
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

private:
	// ---コンポーネント---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	// ---入力関連---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	// ---移動速度関連---
	float DefaultMaxSpeed = 0.0f;

	// ---リロード関連---
	void FinishReload();

	FTimerHandle ReloadTimerHandle;

	// ---スピードバフ管理用---
	float CurrentSpeedMultiplier = 1.0f;
	bool bIsSpeedBuffActive = false;
	FTimerHandle SpeedBuffTimerHandle;

	void ResetSpeedBuff();

	// ---攻撃力バフ管理用---
	FTimerHandle PowerUpTimerHandle;

	void ResetAttackDamage();
};