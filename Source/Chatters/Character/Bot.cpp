// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot.h"


// Sets default values
ABot::ABot()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->MaxHealthPoints = 100;
	this->HealthPoints = this->MaxHealthPoints;
	this->ID = 0;
	this->DisplayName = FString(TEXT(""));

	this->Head = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Head"));

	this->Head->SetupAttachment(this->GetMesh(), NAME_None);
}

// Called when the game starts or when spawned
void ABot::BeginPlay()
{
	Super::BeginPlay();

	this->MaxHealthPoints = 100;
	this->HealthPoints = this->MaxHealthPoints;
	

}

// Called every frame
void ABot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->bMovingToRandomLocation)
	{
		float DistToTarget = FVector::Dist(this->GetActorLocation(), this->RandomLocationTarget);

		if (DistToTarget <= 150.0f)
		{
			this->MoveToRandomLocation();
		}
	}

}

// Called to bind functionality to input
void ABot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool ABot::GetIsAlive()
{
	return this->bAlive;
}

ABotController* ABot::GetAIController()
{
	auto* BotController = this->GetController();

	if (!BotController)
	{
		return nullptr;
	}

	return Cast<ABotController>(BotController);
}

ABot* ABot::CreateBot(UWorld* World, FString NameToSet, uint32 IDToSet, TSubclassOf<ABot> Subclass)
{
	FVector BotPosition(0, float(IDToSet * 300), 100);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*FString::Printf(TEXT("Bot_%d"), IDToSet));
	SpawnParams.bNoFail = true;
	ABot* Bot = World->SpawnActor<ABot>(Subclass, BotPosition, FRotator(0), SpawnParams);

	if (Bot)
	{
		Bot->SpawnDefaultController();
		Bot->DisplayName = NameToSet;
		Bot->ID = IDToSet;

		Bot->MoveToRandomLocation();
	}

	return Bot;
}


void ABot::SetOutfit()
{

}

void ABot::MoveToRandomLocation()
{
	auto* AIController = this->GetAIController();

	if (AIController)
	{

		float XPos = FMath::RandRange(-7400, 7400);
		float YPos = FMath::RandRange(-7400, 7400);

		this->RandomLocationTarget = FVector(XPos, YPos, 97);

		AIController->MoveToLocation(this->RandomLocationTarget);
		bMovingToRandomLocation = true;
	}
}