#include "TurretProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATurretProjectile::ATurretProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 衝突判定用のスフィアコンポーネントをルートとして作成
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(10.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->OnComponentHit.AddDynamic(this, &ATurretProjectile::OnHit);
	RootComponent = CollisionComp;

	// 弾の見た目となるメッシュを作成しコリジョンにあった地
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComp);

	// 弾の移動を制御するプロジェクタイル・ムーブメントコンポーネントの設定
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	// 進行方向を向くようにする
	ProjectileMovement->bRotationFollowsVelocity = true;
	// 重力無効
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// 一定時間経過したら自動で消滅するように設定
	InitialLifeSpan = 3.0f;
}

void ATurretProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 自身やオーナー（発射元のタレット）以外にヒットした場合の処理
	if (OtherActor && (OtherActor != this) && (OtherActor != GetOwner()))
	{
		// プレイヤー等にダメージを適用する
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
		// ヒットしたら弾自身を消滅させる
		Destroy();
	}
}