#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TurretActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;

/**
 * プレイヤーを検知して自動で狙い撃つタレットのクラス
 */
UCLASS()
class UNBUILT_API ATurretActor : public AActor
{
	GENERATED_BODY()

public:
	ATurretActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ---コンポーネント---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TurretMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> DetectionRange;

	// ---戦闘設定---
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float RotationSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundBase* FireSound;

private:
	// 発射用タイマーハンドル
	FTimerHandle FireTimerHandle;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	// 射撃の開始・停止・実効関数
	void StartFiring();
	void StopFiring();
	void Fire();

	// プレイヤーが検知範囲に入ってきたときのオーバーラップ処理
	UFUNCTION()
	void OnDetectionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// プレイヤーが検知範囲から出たときのオーバーラップ終了処理
	UFUNCTION()
	void OnDetectionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};