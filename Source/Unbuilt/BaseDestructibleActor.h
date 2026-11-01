#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemObjects.h"
#include "ItemBase.h"
#include "BaseDestructibleActor.generated.h"

class UNiagaraSystem;
class USoundBase;

/**
 * Chaos物理による破壊、報酬、アイテムドロップを管理する破壊可能オブジェクトのベースクラス
 */
UCLASS()
class UNBUILT_API ABaseDestructibleActor : public AActor
{
	GENERATED_BODY()

public:
	ABaseDestructibleActor();

	// ダメージを受けたときの処理
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	// オブジェクト破壊時の処理
	virtual void HandleDestruction();

	// ---コンポーネント---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGeometryCollectionComponent* GCComponent;

	// ---Chaos物理フィールド---

	UPROPERTY()
	TObjectPtr<URadialFalloff> StrainFalloffField;

	UPROPERTY()
	TObjectPtr<URadialVector> ForceVectorField;

	// ---ステータス---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 ScoreValue = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float XPValue = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float WantedPointValue = 10.0f;

	// ---破壊状態---

	bool bIsDestroyed = false;

	// ---破壊パラメータ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float DestructionRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float StrainMagnitude = 1000000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float LaunchForceMagnitude = 5000000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float ImpulseOffsetZ = -100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float LifeSpanAfterDestruction = 3.0f;

	// ---アイテムドロップ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TArray<TSubclassOf<AItemBase>> DropItemClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ItemDropChance = 0.7f;

	// ランダムなアイテムをスポーン
	void SpawnRandomItem();

	// ---エフェクト---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* DestructionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* DestructionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FVector EffectLocationOffset = FVector(0.0f, 0.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FVector EffectScale = FVector(1.0f, 1.0f, 1.0f);
};