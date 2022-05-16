// Fill out your copyright notice in the Description page of Project Settings.


#include "Animal.h"
#include "AnimalController.h"

// Sets default values
AAnimal::AAnimal()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AAnimalController::StaticClass();
}

// Called when the game starts or when spawned
void AAnimal::BeginPlay()
{
	Super::BeginPlay();
	
	auto* ControllerRef = Cast<AAnimalController>(GetController());

	if (ControllerRef)
	{
		ControllerRef->MoveToRandomLocation();
	}
}

// Called every frame
void AAnimal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


