// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnbuiltProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

/**
 * プレイヤーが発射する弾丸（プロジェクタイル）クラス（通常ヒットおよびChaos破壊に対応）
 */
UCLASS(config=Game)
class AUnbuiltProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

public:
	AUnbuiltProjectile();

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
	
	// ---ChaosDestructionシステム設定---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Destruction")
	float DestructionRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Destruction")
	float DestructionStrength = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Destruction")
	float BulletExpiryDelay = 0.05f;

	// ---戦闘設定---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 50.0f;
};

