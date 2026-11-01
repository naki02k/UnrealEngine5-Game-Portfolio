// Fill out your copyright notice in the Description page of Project Settings.

#include "TurretActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATurretActor::ATurretActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// ダミールートを作成
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// メッシュをルートにアタッチ
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(RootComponent);

	// 検知範囲をルートにアタッチ
	DetectionRange = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionRange"));
	DetectionRange->SetupAttachment(RootComponent);
	DetectionRange->SetSphereRadius(1000.0f);

	// 検知用スフィアが弾丸や攻撃をブロックしないように「Trigger」に設定
	DetectionRange->SetCollisionProfileName(TEXT("Trigger"));
}

void ATurretActor::BeginPlay()
{
	Super::BeginPlay();

	// オーバーラップの開始・終了イベントをバインド
	DetectionRange->OnComponentBeginOverlap.AddDynamic(this, &ATurretActor::OnDetectionOverlapBegin);
	DetectionRange->OnComponentEndOverlap.AddDynamic(this, &ATurretActor::OnDetectionOverlapEnd);
}

void ATurretActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ターゲットが存在する場合、滑らかに方向を合わせる
	if (TargetActor)
	{
		FVector StartLocation = TurretMesh->GetComponentLocation();
		// プレイヤーの中心付近をターゲットにする
		FVector TargetLocation = TargetActor->GetActorLocation() + FVector(0.f, 0.f, 50.f);

		// ターゲットへの方向を計算
		FRotator FullLookAtRotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);

		// Yaw（左右）のみ抽出し、Pitch（上下）とRoll（傾き）を固定して不自然な傾きを防ぐ
		FRotator TargetRotation = FRotator(0.0f, FullLookAtRotation.Yaw - 90.0f, 0.0f);
		FRotator CurrentRotation = TurretMesh->GetComponentRotation();

		// 現在の向きからターゲットの向きへ滑らかに補間して回転
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);

		TurretMesh->SetWorldRotation(NewRotation);
	}
}
void ATurretActor::OnDetectionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// プレイヤー（Pawn）を検知して攻撃開始
	if (OtherActor && OtherActor != this && OtherActor->IsA(APawn::StaticClass()))
	{
		TargetActor = OtherActor;
		StartFiring();
	}
}

void ATurretActor::OnDetectionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// ターゲットが範囲外に出たら追尾と射撃を停止
	if (OtherActor == TargetActor)
	{
		TargetActor = nullptr;
		StopFiring();
	}
}

void ATurretActor::StartFiring()
{
	// タイマーが動いていない場合のみ、FireRate間隔で弾を発射するタイマーをセット
	if (!GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ATurretActor::Fire, FireRate, true);
	}
}

void ATurretActor::StopFiring()
{
	// タイマーをクリアして射撃を止める
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void ATurretActor::Fire()
{
	if (!ProjectileClass) return;

	// タレットメッシュの「MuzzleSocket」から位置と回転を取得
	FVector MuzzleLocation = TurretMesh->GetSocketLocation(TEXT("MuzzleSocket"));
	FRotator MuzzleRotation = TurretMesh->GetSocketRotation(TEXT("MuzzleSocket"));

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	// 弾をスポーン
	GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);

	// 発射音を鳴らす
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation);
	}
}