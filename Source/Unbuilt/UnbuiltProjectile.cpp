// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnbuiltProjectile.h"
#include "BaseDestructibleActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

AUnbuiltProjectile::AUnbuiltProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	CollisionComp->OnComponentHit.AddDynamic(this, &AUnbuiltProjectile::OnHit);

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AUnbuiltProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		// 破壊可能アクター（Destructible）かどうかの判定
		ABaseDestructibleActor* DestructibleActor = Cast<ABaseDestructibleActor>(OtherActor);

		if (DestructibleActor)
		{
			// 破壊用コンポーネントを取得
			UGeometryCollectionComponent* GCComp = Cast<UGeometryCollectionComponent>(OtherComp);

			if (GCComp)
			{
				UE_LOG(LogTemp, Error, TEXT("!!! CHAOS HIT DETECTED !!!"));

				// スリープを強制解除
				GCComp->WakeAllRigidBodies();

				// 衝撃を加える
				GCComp->AddRadialImpulse(
					Hit.Location,
					DestructionRadius,
					DestructionStrength,
					ERadialImpulseFalloff::RIF_Linear,
					true
				);

				// UGameplayStatics::ApplyDamage を経由してダメージを与える
				UGameplayStatics::ApplyDamage(
					DestructibleActor,
					AttackDamage,
					GetInstigatorController(),
					this,
					UDamageType::StaticClass()
				);

				// 弾丸の消去
				SetLifeSpan(BulletExpiryDelay);
				return;
			}
		}

		// 通常の物理オブジェクトへの処理
		if (OtherComp->IsSimulatingPhysics())
		{
			OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		}

		// 通常ヒット時は即座に破棄
		Destroy();
	}
}