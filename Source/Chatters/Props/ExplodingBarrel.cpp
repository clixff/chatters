// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplodingBarrel.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/Bot.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Player/PlayerPawn.h"
#include "../Core/ChattersGameInstance.h"
#include "../Core/ChattersGameSession.h"

// Sets default values
AExplodingBarrel::AExplodingBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	this->DestructibleComponent = CreateDefaultSubobject<UDestructibleComponent>(TEXT("DestructibleMesh"));
	this->DestructibleComponent->SetCanEverAffectNavigation(true);
	this->SetRootComponent(this->DestructibleComponent);

	this->SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Radius"));
	this->SphereComponent->SetupAttachment(this->GetRootComponent());
	this->SphereComponent->SetSphereRadius(this->Radius, false);
	this->SphereComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	this->SphereComponent->SetCanEverAffectNavigation(false);
}

// Called when the game starts or when spawned
void AExplodingBarrel::BeginPlay()
{
	AActor::BeginPlay();

	UChattersGameSession* GameSession = UChattersGameSession::Get();

	if (GameSession)
	{
		GameSession->ExplodingBarrels.Add(this);
	}
	
}

// Called every frame
void AExplodingBarrel::Tick(float DeltaTime)
{
	AActor::Tick(DeltaTime);

}

void AExplodingBarrel::Explode(ABot* BotCauser)
{
	if (this->bCanExplode)
	{
		if (this->ParticleSystem)
		{
			FTransform ParticleTransform;
			ParticleTransform.SetLocation(this->GetActorLocation());
			ParticleTransform.SetScale3D(this->ParticleScale);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), this->ParticleSystem, ParticleTransform, true);
		}

		if (this->ExplodingSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), this->ExplodingSound, this->GetActorLocation(), FMath::RandRange(0.5f, 0.75f));
		}

		UChattersGameSession* GameSession = UChattersGameSession::Get();

		if (GameSession)
		{
			for (int32 i = 0; i < GameSession->AvailableExplodingBarrels.Num(); i++)
			{
				auto* Barrel = GameSession->AvailableExplodingBarrels[i];
				if (Barrel == this)
				{
					GameSession->AvailableExplodingBarrels.RemoveAt(i, 1, true);
					break;
				}
			}
		}

		FVector BarrelLocation = this->GetActorLocation();
		TArray<ABot*> BotsInRadius = this->GetBotsInRadius();

		for (int32 i = 0; i < BotsInRadius.Num(); i++)
		{
			auto* Bot = BotsInRadius[i];

			if (Bot && Bot->bAlive)
			{
				FVector BotLocation = Bot->GetActorLocation();
				float Dist = FVector::Dist(BarrelLocation, BotLocation);
				
				float DistScale = FMath::Clamp((Dist / this->Radius), 0.0f, 1.0f);

				int32 Damage = MaxDamage * DistScale;

				FVector ImpulseVector = UKismetMathLibrary::FindLookAtRotation(BarrelLocation, BotLocation).Vector() * (50000.0f);

				Bot->ApplyDamage(Damage, BotCauser, EWeaponType::Explosion, ImpulseVector, BotLocation);
			}
		}

		if (this->DestructibleComponent)
		{
			this->DestructibleComponent->ApplyDamage(1, this->GetActorLocation(), FVector(0.0f, 0.0f, 1.0f), 10000.0f);
			this->DestructibleComponent->SetCanEverAffectNavigation(false);
		}

		this->bCanExplode = false;
	}
}

TArray<ABot*> AExplodingBarrel::GetBotsInRadius()
{
	TArray<ABot*> Bots;

	auto* GameSession = UChattersGameSession::Get();

	FVector BarrelLocation = this->GetActorLocation();

	if (GameSession)
	{
		for (int32 i = 0; i < GameSession->AliveBots.Num(); i++)
		{
			auto* Bot = GameSession->AliveBots[i];

			if (Bot)
			{
				float Dist = FVector::Dist(BarrelLocation, Bot->GetActorLocation());

				if (Dist <= this->Radius)
				{
					Bots.Add(Bot);
				}
			}
		}
	}

	return Bots;
}

#if WITH_EDITOR
void AExplodingBarrel::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName != NAME_None)
	{
		if (PropertyName == TEXT("Radius"))
		{
			if (this->SphereComponent)
			{
				this->SphereComponent->SetSphereRadius(this->Radius);
			}
		}
	}
}
#endif
