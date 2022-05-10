// Fill out your copyright notice in the Description page of Project Settings.


#include "Train.h"
#include "../Character/Bot.h"
#include "../Core/ChattersGameSession.h"
#include "../Player/PlayerPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ATrain::ATrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	this->SetRootComponent(Root);

	this->SKMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	this->SKMesh->SetupAttachment(Root);

	this->Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	this->Collision->SetupAttachment(SKMesh);

	this->Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	this->Spline->SetupAttachment(Root);

	this->Sound = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	this->Sound->SetupAttachment(SKMesh);
}

// Called when the game starts or when spawned
void ATrain::BeginPlay()
{
	Super::BeginPlay();

	auto* GameSession = UChattersGameSession::Get();

	if (GameSession)
	{
		GameSession->TrainRef = this;
	}

	this->SKMesh->SetHiddenInGame(true, true);
}

// Called every frame
void ATrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bActivated)
	{
		this->PathValue += DeltaTime * Speed;

		this->PathValue = FMath::Clamp(PathValue, 0.0f, 1.0f);

		FVector Location = FMath::Lerp(Start, End, PathValue);

		this->SKMesh->SetWorldLocation(Location);

		if (PathValue == 1.0f)
		{
			this->bActivated = false;

			if (this->Sound && this->Sound->Sound)
			{
				this->Sound->Stop();
			}

			this->SetActorHiddenInGame(true);

			this->SKMesh->bPauseAnims = true;
		}
	}
}

void ATrain::Activate()
{
	this->PathValue = 0.0f;
	this->bActivated = true;

	this->Start = this->Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	this->End = this->Spline->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);

	if (this->Sound && this->Sound->Sound)
	{
		this->Sound->Play();
	}

	this->SKMesh->SetHiddenInGame(false, true);
	this->SKMesh->bPauseAnims = false;
}

void ATrain::TrainCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto* Bot = Cast<ABot>(OtherActor);

	if (!Bot || !Bot->bAlive)
	{
		return;
	}

	FVector HitLocation = Bot->GetActorLocation();

	FVector ImpulseVector = UKismetMathLibrary::FindLookAtRotation(SKMesh->GetComponentLocation(), HitLocation).Vector() * this->ImpulseForce;

	Bot->ApplyDamage(100, Bot, EWeaponType::Train, ImpulseVector, HitLocation);

	if (this->HitSound)
	{
		auto* PlayerPawn = APlayerPawn::Get();

		if (PlayerPawn)
		{
			float Distance = PlayerPawn->GetDistanceFromCamera(HitLocation);

			if (Distance < 5000.0f)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), this->HitSound, HitLocation, 1.0f);
			}
		}
	}
}


