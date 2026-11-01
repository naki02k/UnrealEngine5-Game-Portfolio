#include "ItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 接触検知用のSphereCollisionをルートに設定
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->SetSphereRadius(60.0f);

	// コリジョン設定（Triggerとして動作）
	SphereCollision->SetCollisionProfileName(TEXT("Trigger"));

	// メッシュの設定
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // メッシュ自体には当たり判定を作らない

	// デフォルト値
	ItemName = FText::FromString(TEXT("Default Item"));
	ItemDescription = FText::FromString(TEXT("Item Description"));
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	// 接触イベントのバインド
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AItemBase::OnOverlapBegin);

	// 開始時にメッシュの相対位置を保存
	if (ItemMesh)
	{
		InitialMeshLocation = ItemMesh->GetRelativeLocation();
	}
}

void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ItemMesh)
	{
		RunningTime += DeltaTime;
		// 浮遊処理
		float DeltaZ = FMath::Sin(RunningTime * FloatFrequency) * FloatAmplitude;

		FVector NewLocation = InitialMeshLocation;
		NewLocation.Z = DeltaZ;
		ItemMesh->SetRelativeLocation(NewLocation);
	}
}

void AItemBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 自身以外のアクター（プレイヤー等）と接触した場合
	if (OtherActor && OtherActor != this)
	{
		// 拾った時の処理を実行
		OnPickUp(OtherActor);
	}
}

void AItemBase::OnPickUp_Implementation(AActor* Picker)
{
	if (!Picker) return;

	//  効果を適用（子クラスやBPでオーバーライドされた処理が走る）
	ApplyEffect(Picker);
	// サウンドが設定されていればアイテムの位置で再生する
	if (PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickUpSound, GetActorLocation());
	}

	// 音やパーティクルなどの演出を実行（BP側で設定されている場合）
	PlayPickUpEffects();
	// 即時消費アイテムのため自身を破棄
	Destroy();
}

void AItemBase::ApplyEffect_Implementation(AActor* TargetActor)
{
	// 基底クラスではログ出力のみ（子クラス側で具体的なバフや時間延長の処理を書く）
	UE_LOG(LogTemp, Log, TEXT("%s effect applied to %s"), *ItemName.ToString(), TargetActor ? *TargetActor->GetName() : TEXT("Unknown"));
}