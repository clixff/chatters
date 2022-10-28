// Fill out your copyright notice in the Description page of Project Settings.


#include "Robot.h"
#include "RobotController.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Bot.h"


// Sets default values
ARobot::ARobot()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = ARobotController::StaticClass();

	GetCapsuleComponent()->SetHiddenInGame(false);
	GetCapsuleComponent()->SetVisibility(true);
	GetCapsuleComponent()->SetCanEverAffectNavigation(false);

	GetMesh()->SetCanEverAffectNavigation(false);

#if WITH_EDITOR
	GetArrowComponent()->SetHiddenInGame(false);
	GetArrowComponent()->SetVisibility(true);
#endif
	
	GetCharacterMovement()->bUseRVOAvoidance = true;

	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetGenerateOverlapEvents(false);

	DestructibleComponent = CreateDefaultSubobject<UDestructibleComponent>(TEXT("DestructibleComponent"));
	DestructibleComponent->SetupAttachment(GetRootComponent());
	DestructibleComponent->SetHiddenInGame(true);
	DestructibleComponent->SetVisibility(false);
	DestructibleComponent->SetCanEverAffectNavigation(false);
	DestructibleComponent->SetGenerateOverlapEvents(false);

	DestructibleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ARobot::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnDefaultController();

	TArray<USceneComponent*> MeshChildren;

	GetMesh()->GetChildrenComponents(false, MeshChildren);

	GunPoints.Empty();

	if (MeshChildren.Num())
	{
		for (auto* Child : MeshChildren)
		{
			if (Child->ComponentHasTag(TEXT("gun")))
			{
				GunPoints.Add(Child);
			}
		}
	}
}

// Called every frame
void ARobot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDestroyed)
	{
		DestructibleTimer.Add(DeltaTime);

		if (DestructibleTimer.IsEnded())
		{
			DestructibleComponent->ReregisterComponent();
			DestructibleComponent->SetHiddenInGame(true);
			DestructibleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

// Called to bind functionality to input
void ARobot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ARobot::MoveToNewLocation(FVector NewLocation)
{
	auto* RobotController = Cast<ARobotController>(GetController());

	if (RobotController)
	{
		RobotController->MoveToNewLocation(NewLocation);
	}
}

void ARobot::StopMovement()
{
	auto* RobotController = Cast<ARobotController>(GetController());

	if (RobotController)
	{
		RobotController->StopMovement();
	}
}

void ARobot::OnDead()
{
	bDestroyed = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMesh()->SetVisibility(false, true);

	if (BotOwner)
	{
		BotOwner->GetMesh()->SetVisibility(false, true);
		BotOwner->SetActorHiddenInGame(true);
	}

	if (ParticleSystem)
	{
		FTransform ParticleTransform;
		ParticleTransform.SetLocation(BotOwner->GetActorLocation());
		ParticleTransform.SetScale3D(ParticleScale);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleSystem, ParticleTransform, true);
	}

	if (ExplodingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplodingSound, GetActorLocation(), FMath::RandRange(0.5f, 0.75f));
	}

	GetCharacterMovement()->bUseRVOAvoidance = false;

	DestructibleComponent->SetWorldLocation(GetMesh()->GetComponentLocation());
	DestructibleComponent->SetWorldRotation(GetMesh()->GetComponentRotation());

	DestructibleComponent->SetVisibility(true);
	DestructibleComponent->SetHiddenInGame(false);

	DestructibleComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	DestructibleComponent->ApplyDamage(ExplosionDamage, GetActorLocation(), ExplosionVector, ExplosionForce);
	DestructibleComponent->SetCanEverAffectNavigation(false);
}

void ARobot::OnRespawn()
{
	DestructibleComponent->ReregisterComponent();

	DestructibleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DestructibleComponent->SetVisibility(false);
	DestructibleComponent->SetHiddenInGame(true);

	GetCharacterMovement()->bUseRVOAvoidance = true;

	bDestroyed = false;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	GetMesh()->SetVisibility(true, true);

	if (BotOwner)
	{
		BotOwner->GetMesh()->SetVisibility(true, true);
		BotOwner->SetActorHiddenInGame(false);

		BotOwner->SetActorRelativeRotation(CharacterTransform.GetRotation().Rotator());
		BotOwner->SetActorRelativeLocation(CharacterTransform.GetLocation());
	}
}

FVector ARobot::GetGunPosition()
{
	if (GunPoints.Num())
	{
		int32 RandNumber = FMath::RandRange(0, GunPoints.Num() - 1);

		return GunPoints[RandNumber]->GetComponentLocation();
	}

	return GetActorLocation();
}

