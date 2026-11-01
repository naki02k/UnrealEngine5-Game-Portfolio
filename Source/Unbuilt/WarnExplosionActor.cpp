#include "WarnExplosionActor.h"

#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"

AWarnExplosionActor::AWarnExplosionActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// ルートコンポーネント
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// 警告デカール
	WarningDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("WarningDecal"));
	WarningDecal->SetupAttachment(RootComponent);
	WarningDecal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	WarningDecal->DecalSize = FVector(128.0f, 300.0f, 300.0f);
}

void AWarnExplosionActor::BeginPlay()
{
	Super::BeginPlay();

	if (WarningDecal && WarningDecal->GetDecalMaterial())
	{
		DynamicDecalMaterial = WarningDecal->CreateDynamicMaterialInstance();
	}

	// 0以下の場合は即座に爆発
	if (ExplosionDuration <= 0.0f)
	{
		Explode();
	}
}

void AWarnExplosionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// すでに爆発している場合は処理しない
	if (bHasExploded)
	{
		return;
	}

	ElapsedTime += DeltaTime;

	// 爆発までの進行率を計算
	const float Progress = FMath::Clamp(
		ElapsedTime / ExplosionDuration,
		0.0f,
		1.0f
	);

	// デカールのマテリアルパラメータを更新
	if (DynamicDecalMaterial)
	{
		DynamicDecalMaterial->SetScalarParameterValue(
			TEXT("Progress"),
			Progress
		);
	}

	// 指定時間経過後に爆発
	if (ElapsedTime >= ExplosionDuration)
	{
		Explode();
	}
}

void AWarnExplosionActor::Explode()
{
	// 二重実行防止
	if (bHasExploded)
	{
		return;
	}

	bHasExploded = true;

	// 爆発後はTickを停止
	SetActorTickEnabled(false);

	const FVector ExplosionLocation = GetActorLocation();

	// 爆発エフェクト
	if (ExplosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionEffect,
			ExplosionLocation
		);
	}

	// 爆発音
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ExplosionSound,
			ExplosionLocation
		);
	}

	// プレイヤーだけをダメージ対象にする
	ACharacter* PlayerCharacter =
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (IsValid(PlayerCharacter))
	{
		const float DistanceToPlayer =
			FVector::Dist(
				ExplosionLocation,
				PlayerCharacter->GetActorLocation()
			);

		if (DistanceToPlayer <= ExplosionRadius)
		{
			UGameplayStatics::ApplyDamage(
				PlayerCharacter,
				ExplosionDamage,
				GetInstigatorController(),
				this,
				UDamageType::StaticClass()
			);
		}
	}

	// 爆発アクターを削除
	Destroy();
}