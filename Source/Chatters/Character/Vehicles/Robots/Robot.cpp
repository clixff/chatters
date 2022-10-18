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
}

// Called when the game starts or when spawned
void ARobot::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnDefaultController();
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

	DestructibleComponent->ApplyDamage(1, BotOwner->GetActorLocation(), FVector(0.0f, 0.0f, 1.0f), ExplosionForce);
	DestructibleComponent->SetCanEverAffectNavigation(false);
}

