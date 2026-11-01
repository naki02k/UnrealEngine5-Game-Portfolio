#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USoundBase;

/**
 * すべてのアイテムのベースとなる親クラス（取得判定、アニメーション、エフェクト等管理）
 */
UCLASS()
class UNBUILT_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ---コンポーネント---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Components")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Components")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	//---アイテムデータ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Data")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Data")
	FText ItemDescription;

	// ---サウンド---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Sound")
	TObjectPtr<USoundBase> PickUpSound;

	// プレイヤー等が接触したときのオーバーラップ開始処理
	UFUNCTION()
	virtual void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// アイテムの買うかを適用する関数
	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void ApplyEffect(AActor* TargetActor);
	virtual void ApplyEffect_Implementation(AActor* TargetActor);

	// 取得時のエフェクト再生
	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Visual")
	void PlayPickUpEffects();

public:	
	virtual void Tick(float DeltaTime)override;

	// ---アニメーション設定---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Animation")
	float FloatAmplitude = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Animation")
	float FloatFrequency = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Animation")
	float RotationSpeed = 90.0f;

	// アイテム取得時のメイン処理
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void OnPickUp(AActor* Picker);
	virtual void OnPickUp_Implementation(AActor* Picker);

	// ゲッター系
	FORCEINLINE FText GetItemName()const { return ItemName; }
	FORCEINLINE UStaticMeshComponent* GetItemMesh()const { return ItemMesh; }

private:
	// アニメーション計算用変数
	FVector InitialMeshLocation;
	float RunningTime = 0.0f;
};
