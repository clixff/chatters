// Fill out your copyright notice in the Description page of Project Settings.


#include "ScifiBomber.h"
#include "../../../Core/ChattersGameSession.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AScifiBomber::AScifiBomber()
{
	MainComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(MainComponent);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(MainComponent);

	ProjectileOutPosition = CreateDefaultSubobject<UArrowComponent>(TEXT("ProjectileOutPos"));
	ProjectileOutPosition->SetupAttachment(StaticMesh);

	bActivateEveryRound = true;

	EngineSound = CreateDefaultSubobject<UAudioComponent>(TEXT("EngienSound"));
	EngineSound->SetupAttachment(StaticMesh);

}

void AScifiBomber::Activate()
{
	AActivateProp::Activate();

	bFlying = false;
	bAttacked = false;
	TimeoutTimer.Reset();
	FlyTimer.Reset();
	ProjectileTimer.Reset();

	if (ProjectileActor)
	{
		ProjectileActor->Destroy();
		ProjectileActor = nullptr;
	}


}

void AScifiBomber::Deactivate()
{
	AActivateProp::Deactivate();

	StopFlying();
}

void AScifiBomber::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bActivated)
	{
		if (bFlying)
		{
			FlyTick(DeltaTime);
		}
		else
		{
			TimeoutTimer.Add(DeltaTime);
			if (TimeoutTimer.IsEnded())
			{
				StartFlying();
				TimeoutTimer.Reset();
			}
		}
	}

	if (bAttacked)
	{
		ProjectileTick(DeltaTime);
	}
}

void AScifiBomber::ProjectileTick(float DeltaTime)
{
	if (ProjectileTimer.IsEnded())
	{
		return;
	}

	if (!ProjectileActor)
	{
		return;
	}

	ProjectileTimer.Add(DeltaTime);

	float TimeScale = FMath::Clamp((ProjectileTimer.Current / ProjectileTimer.Max), 0.0f, 1.0f);

	FVector NewLocation = FMath::Lerp(ProjectileStartLocation, ProjectileEndLocation, TimeScale);

	ProjectileActor->SetActorLocation(NewLocation);

	if (ProjectileTimer.IsEnded())
	{
		ProjectileActor->Destroy();
		ProjectileActor = nullptr;

		auto* GameSession = UChattersGameSession::Get();

		if (!GameSession)
		{
			return;
		}

		GameSession->AddExplosionAtLocation(ProjectileEndLocation, ExplosionParticle, ExplosionParticleScale, ExplosionSound, ExplosionRadius, ImpulseForce, EWeaponType::Bomber);
	}
}

void AScifiBomber::Attack()
{
	bAttacked = true;

	ProjectileStartLocation = ProjectileOutPosition->GetComponentLocation();

	ProjectileEndLocation = FVector(AttackLocation.X, AttackLocation.Y, -100.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(HitResult, ProjectileStartLocation, ProjectileEndLocation, ECollisionChannel::ECC_GameTraceChannel3, Params);

	if (HitResult.bBlockingHit)
	{
		ProjectileEndLocation = HitResult.ImpactPoint;
	}

	ProjectileTimer.Max = FVector::Dist(ProjectileStartLocation, ProjectileEndLocation) / ProjectileSpeed;

	if (ProjectileActorClass)
	{
		ProjectileActor = GetWorld()->SpawnActor<AActor>(ProjectileActorClass, ProjectileStartLocation, FRotator::ZeroRotator);
	}

	ProjectileTimer.Reset();
}

void AScifiBomber::FlyTick(float DeltaTime)
{
	FlyTimer.Add(DeltaTime);

	float TimeScale = FMath::Clamp((FlyTimer.Current / FlyTimer.Max), 0.0f, 1.0f);

	FVector Location = StaticMesh->GetComponentLocation();

	Location.X = FMath::Lerp(StartLocation.X, EndLocation.X, TimeScale);

	StaticMesh->SetWorldLocation(Location);

	if (!bAttacked && Location.X >= AttackLocation.X - ProjectileOffset)
	{
		Attack();
	}

	if (FlyTimer.IsEnded())
	{
		StopFlying();
	}
}

void AScifiBomber::StartFlying()
{
	bFlying = true;

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

	StaticMesh->SetVisibility(true, true);

	StartLocation.X = XRange.GetLowerBoundValue() - FlyOffset;
	//StartLocation.Y = FMath::RandRange(YRange.GetLowerBoundValue(), YRange.GetUpperBoundValue());

	EndLocation.X = XRange.GetUpperBoundValue() + FlyOffset;

	AttackLocation = FVector2D(Bot->GetActorLocation());

	const float OffsetMax = 1500.0f;
	FVector2D Offset;
	Offset.X = FMath::RandRange(-OffsetMax, OffsetMax);
	Offset.Y = FMath::RandRange(-OffsetMax, OffsetMax);

	AttackLocation += Offset;

	StartLocation.Y = AttackLocation.Y;
	EndLocation.Y = StartLocation.Y;
	bAttacked = false;

	StaticMesh->SetWorldLocation(FVector(StartLocation, FlyingHeight));

	FlyTimer.Max = FVector2D::Distance(StartLocation, EndLocation) / FlySpeed;
	FlyTimer.Reset();

	if (EngineSound)
	{
		EngineSound->SetVolumeMultiplier(1.0f);
	}
}

void AScifiBomber::StopFlying()
{
	bFlying = false;

	StaticMesh->SetVisibility(false, true);

	TimeoutTimer.Reset();

	bAttacked = false;

	EngineSound->SetVolumeMultiplier(0.0f);
}

void AScifiBomber::BeginPlay()
{
	Super::BeginPlay();

	StaticMesh->SetVisibility(false, true);
	EngineSound->SetVolumeMultiplier(0.0f);
}
