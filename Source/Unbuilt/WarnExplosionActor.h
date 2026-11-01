#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WarnExplosionActor.generated.h"

class UDecalComponent;
class UMaterialInstanceDynamic;
class UNiagaraSystem;
class USoundBase;

/**
 * 警告デカールを表示した後、指定時間経過後にプレイヤーへダメージを与える爆発アクター
 */
UCLASS()
class UNBUILT_API AWarnExplosionActor : public AActor
{
	GENERATED_BODY()

public:
	AWarnExplosionActor();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float ExplosionRadius = 300.0f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDecalComponent> WarningDecal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float ExplosionDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float ExplosionDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<USoundBase> ExplosionSound;

private:
	float ElapsedTime = 0.0f;
	bool bHasExploded = false;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicDecalMaterial;

	// 爆発処理を実行
	void Explode();
};