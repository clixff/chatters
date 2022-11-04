// Fill out your copyright notice in the Description page of Project Settings.


#include "ScifiWalker.h"
#include "../../../Core/ChattersGameSession.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AScifiWalker::AScifiWalker()
{
	MainComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(MainComponent);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(MainComponent);

	ProjectileOutPosition = CreateDefaultSubobject<UArrowComponent>(TEXT("ProjectileOutPos"));
	ProjectileOutPosition->SetupAttachment(StaticMesh);

	bActivateEveryRound = true;
}

void AScifiWalker::Activate()
{
	AActivateProp::Activate();
	Timer.Reset();
}

void AScifiWalker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bActivated)
	{
		Timer.Add(DeltaTime);

		if (Timer.IsEnded())
		{
			Attack();
			Timer.Reset();
		}
	}


	ProjectileTick(DeltaTime);
}

void AScifiWalker::ProjectileTick(float DeltaTime)
{
	if (!bProjectileMoving || !ProjectileActor)
	{
		return;
	}

	ProjectileTimer.Add(DeltaTime);

	float TimerValue =  (ProjectileTimer.Current / ProjectileTimer.Max);
	TimerValue = FMath::Clamp(TimerValue, 0.0f, 1.0f);

	FVector NewLocation = FMath::Lerp(ProjectileStartLocation, TargetLocation, TimerValue);

	ProjectileActor->SetActorLocation(NewLocation);


	if (ProjectileTimer.IsEnded())
	{
		ProjectileTimer.Reset();
		ProjectileActor->Destroy();
		ProjectileActor = nullptr;
		bProjectileMoving = false;


		auto* GameSession = UChattersGameSession::Get();

		if (!GameSession)
		{
			return;
		}

		GameSession->AddExplosionAtLocation(TargetLocation, ExplosionParticle, ExplosionParticleScale, ExplosionSound, ExplosionRadius, ImpulseForce, EWeaponType::Walker);
	}
}

void AScifiWalker::Attack()
{
	auto* GameSession = UChattersGameSession::Get();

	if (!GameSession)
	{
		return;
	}

	auto AliveBotsCopy = GameSession->AliveBots;

	if (GameSession->GameModeType == ESessionGameMode::Zombie)
	{
		AliveBotsCopy.SetNum(GameSession->AliveBots.Num() + GameSession->Zombies.Num());

		for (int32 i = 0; i < GameSession->Zombies.Num(); i++)
		{
			AliveBotsCopy[GameSession->AliveBots.Num() + i] = GameSession->Zombies[i];
		}
	}

	if (!AliveBotsCopy.Num())
	{
		return;
	}

	auto* Bot = AliveBotsCopy[FMath::RandRange(0, AliveBotsCopy.Num() - 1)];

	if (!Bot)
	{
		return;
	}

	ProjectileStartLocation = ProjectileOutPosition->GetComponentLocation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	TargetLocation = Bot->GetActorLocation();

	FVector Offset;
	const float RandOffsetMax = 500.0f;
	Offset.X = FMath::RandRange(-RandOffsetMax, RandOffsetMax);
	Offset.Y = FMath::RandRange(-RandOffsetMax, RandOffsetMax);
	Offset.Z = FMath::RandRange(-RandOffsetMax, RandOffsetMax);

	TargetLocation += Offset;

	GetWorld()->LineTraceSingleByChannel(HitResult, ProjectileStartLocation, TargetLocation, ECollisionChannel::ECC_GameTraceChannel3, Params);

	if (HitResult.bBlockingHit)
	{
		TargetLocation = HitResult.ImpactPoint;
	}

	bProjectileMoving = true;
	ProjectileTimer.Reset();
	ProjectileTimer.Max = FVector::Dist(ProjectileStartLocation, TargetLocation) / ProjectileSpeed;

	if (ProjectileTimer.Max > Timer.Max)
	{
		ProjectileTimer.Max = Timer.Max;
	}

	if (ProjectileActor)
	{
		ProjectileActor->Destroy();
		ProjectileActor = nullptr;
	}

	if (ProjectileActorClass)
	{
		FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(ProjectileStartLocation, TargetLocation);
		ProjectileActor = GetWorld()->SpawnActor<AActor>(ProjectileActorClass, ProjectileStartLocation, Rotation);
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, ProjectileStartLocation, FMath::RandRange(0.8f, 1.0f));
	}
}
