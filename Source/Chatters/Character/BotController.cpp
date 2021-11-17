// Fill out your copyright notice in the Description page of Project Settings.


#include "BotController.h"
#include "Bot.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

DECLARE_CYCLE_STAT(TEXT("Bot Controller Tick Time"), STAT_StatsBotControllerTick, STATGROUP_BOTS);


ABotController::ABotController()
{

}

ABotController::~ABotController()
{

}

void ABotController::MoveToNewLocation(FVector Location)
{
	auto* PawnObject = this->GetPawn();
	FVector PawnLocation = FVector(0.0f);
	
	if (PawnObject)
	{
		PawnLocation = PawnObject->GetActorLocation();
	}

	float Distance = FVector::Dist(PawnLocation, Location);

	//UE_LOG(LogTemp, Display, TEXT("[ABotController] Moving bot from %s to %s. Distance: %f m"), *(PawnLocation.ToString()), *(Location.ToString()), Distance);

	auto MoveBotToLocation = [this](FVector LocationToMove)
	{
		return this->MoveToLocation(LocationToMove, 10.0f, false, true, true, true);
	};

	auto RequestResult = MoveBotToLocation(Location);

	if (RequestResult == EPathFollowingRequestResult::Type::Failed)
	{
		FVector NewLocation;
		bool bFoundAlternativeLocation = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this->GetWorld(), Location, NewLocation, 250.0f);

		if (bFoundAlternativeLocation)
		{
			MoveBotToLocation(NewLocation);
		}

		ABot* Bot = Cast<ABot>(PawnObject);

		if (Bot)
		{
			UE_LOG(LogTemp, Error, TEXT("[ABotController] Failed to move bot %s. Target: %s. bFoundAlternative: %d"), *Bot->DisplayName, *Location.ToString(), bFoundAlternativeLocation);
		}
	}


	//UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Location);
}

void ABotController::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_StatsBotControllerTick);
	Super::Tick(DeltaTime);
}
