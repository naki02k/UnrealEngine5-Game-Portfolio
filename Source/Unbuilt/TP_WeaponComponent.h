#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "TP_WeaponComponent.generated.h"

class AUnbuiltCharacter;
class AScoreManager;

/**
 * 武器の装備、発射処理、連射レート、マルチショット、ダメージ計算等を管理するコンポーネントクラス
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNBUILT_API UTP_WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AUnbuiltProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector MuzzleOffset;

	/** 発射間隔(秒)。値が小さいほど連射が速い */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponStats")
	float FireRate;

	/** 1回の発射で生成する弾の数 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponStats")
	int32 ProjectileCount;

	/** 拡散角度(度) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponStats")
	float SpreadAngle;

	// [追加] 発射間隔の下限(秒)。これより速くは撃てない
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats", meta = (ClampMin = "0.01"))
	float MinFireInterval = 0.1f;

	// [追加] true: 押しっぱなしで連射 / false: 1クリック1発
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats")
	bool bIsAutomatic = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponStats")
	float BaseDamage = 50.0f;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	/** Sets default values for this component's properties */
	UTP_WeaponComponent();

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool AttachWeapon(AUnbuiltCharacter* TargetCharacter);

	/** Make the weapon Fire a Projectile */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

protected:
	virtual void BeginPlay() override;

	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** The Character holding this weapon*/
	UPROPERTY()
	AUnbuiltCharacter* Character;

	UPROPERTY()
	TObjectPtr<AScoreManager> CachedScoreManager;

	// 連射制御用タイマーハンドルとフラグ
	FTimerHandle FireTimerHandle;

	bool bCanFire;

	// 連射間隔後に再び発射可能にする関数
	void ResetFire();
};