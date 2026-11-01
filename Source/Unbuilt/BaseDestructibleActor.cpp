#include "BaseDestructibleActor.h"
#include "ScoreManager.h"
#include "UnbuiltCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Field/FieldSystemObjects.h"

ABaseDestructibleActor::ABaseDestructibleActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// ルートコンポーネント
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Chaos Geometry Collection
	GCComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
	GCComponent->SetupAttachment(RootComponent);

	// 初期状態では物理シミュレーションを停止
	GCComponent->SetSimulatePhysics(false);
	GCComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ABaseDestructibleActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	if (GCComponent)
	{
		GCComponent->SetSimulatePhysics(false);
		GCComponent->PutAllRigidBodiesToSleep();
	}
}

float ABaseDestructibleActor::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDestroyed)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage <= 0.0f)
	{
		return ActualDamage;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		HandleDestruction();
	}

	return ActualDamage;
}

void ABaseDestructibleActor::HandleDestruction()
{
	if (bIsDestroyed || !GCComponent)
	{
		return;
	}

	bIsDestroyed = true;

	// Chaos物理を有効化
	GCComponent->SetCollisionProfileName(TEXT("Destructible"));
	GCComponent->SetSimulatePhysics(true);
	GCComponent->WakeAllRigidBodies();

	// Geometry Collectionのクラスターを破壊
	GCComponent->CrumbleCluster(0);

	// 破壊の中心位置
	const FVector DestructionLocation =
		GetActorLocation() + FVector(0.0f, 0.0f, ImpulseOffsetZ);

	// -------------------------
	// Chaos Strain Field
	// -------------------------

	if (!StrainFalloffField)
	{
		StrainFalloffField = NewObject<URadialFalloff>(this);
	}

	if (StrainFalloffField)
	{
		const float ExtremeStrain = StrainMagnitude * 100.0f;

		StrainFalloffField->SetRadialFalloff(
			ExtremeStrain,
			0.0f,
			ExtremeStrain,
			0.0f,
			DestructionRadius,
			DestructionLocation,
			EFieldFalloffType::Field_FallOff_None
		);

		GCComponent->ApplyPhysicsField(
			true,
			EGeometryCollectionPhysicsTypeEnum::Chaos_ExternalClusterStrain,
			nullptr,
			StrainFalloffField
		);
	}

	// -------------------------
	// Chaos Force Field
	// -------------------------

	if (!ForceVectorField)
	{
		ForceVectorField = NewObject<URadialVector>(this);
	}

	if (ForceVectorField)
	{
		ForceVectorField->SetRadialVector(
			LaunchForceMagnitude,
			DestructionLocation
		);

		GCComponent->ApplyPhysicsField(
			true,
			EGeometryCollectionPhysicsTypeEnum::Chaos_LinearForce,
			nullptr,
			ForceVectorField
		);
	}

	// エフェクト・サウンドの再生位置
	const FVector EffectLocation =
		GetActorLocation() + EffectLocationOffset;

	if (DestructionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DestructionEffect,
			EffectLocation,
			FRotator::ZeroRotator,
			EffectScale
		);
	}

	if (DestructionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DestructionSound,
			EffectLocation
		);
	}

	// タイトル画面以外で報酬処理
	if (UWorld* World = GetWorld())
	{
		if (!World->GetMapName().Contains(TEXT("Title")))
		{
			AActor* ManagerActor =
				UGameplayStatics::GetActorOfClass(
					World,
					AScoreManager::StaticClass()
				);

			if (AScoreManager* ScoreManager =
				Cast<AScoreManager>(ManagerActor))
			{
				ScoreManager->AddScore(ScoreValue);
				ScoreManager->AddWantedPoint(WantedPointValue);
			}

			ACharacter* PlayerCharacter =
				UGameplayStatics::GetPlayerCharacter(
					World,
					0
				);

			if (AUnbuiltCharacter* Player =
				Cast<AUnbuiltCharacter>(PlayerCharacter))
			{
				Player->AddExperience(XPValue);
			}

			SpawnRandomItem();
		}
	}

	// 指定時間後に破壊オブジェクトを削除
	SetLifeSpan(LifeSpanAfterDestruction);
}

void ABaseDestructibleActor::SpawnRandomItem()
{
	UWorld* World = GetWorld();

	if (!World || DropItemClasses.IsEmpty())
	{
		return;
	}

	// アイテムドロップ確率
	if (FMath::FRand() > ItemDropChance)
	{
		return;
	}

	// ランダムにアイテムを選択
	const int32 RandomIndex =
		FMath::RandRange(0, DropItemClasses.Num() - 1);

	const TSubclassOf<AItemBase> SelectedItemClass =
		DropItemClasses[RandomIndex];

	if (!SelectedItemClass)
	{
		return;
	}

	// ドロップ位置
	const FVector DropLocation =
		GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	World->SpawnActor<AItemBase>(
		SelectedItemClass,
		DropLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);
}