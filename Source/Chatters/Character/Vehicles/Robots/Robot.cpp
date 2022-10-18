// Fill out your copyright notice in the Description page of Project Settings.


#include "Robot.h"
#include "RobotController.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"



// Sets default values
ARobot::ARobot()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = ARobotController::StaticClass();

	GetCapsuleComponent()->SetHiddenInGame(false);
	GetCapsuleComponent()->SetVisibility(true);

#if WITH_EDITOR
	GetArrowComponent()->SetHiddenInGame(false);
	GetArrowComponent()->SetVisibility(true);
#endif
	
	GetCharacterMovement()->bUseRVOAvoidance = true;

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

