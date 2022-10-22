// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotController.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

void ARobotController::MoveToNewLocation(FVector Location)
{
	auto* PawnObject = this->GetPawn();
	FVector PawnLocation = FVector(0.0f);

	if (PawnObject)
	{
		PawnLocation = PawnObject->GetActorLocation();
	}

	float Distance = FVector::Dist(PawnLocation, Location);

	//UE_LOG(LogTemp, Display, TEXT("[ARobotController] Moving robot from %s to %s. Distance: %f m"), *(PawnLocation.ToString()), *(Location.ToString()), Distance);

	auto MoveBotToLocation = [this](FVector LocationToMove)
	{
		return this->MoveToLocation(LocationToMove, 50.0f, false, true, true, true);
	};

	auto RequestResult = MoveBotToLocation(Location);

	if (RequestResult == EPathFollowingRequestResult::Type::Failed)
	{
		FVector NewLocation;
		bool bFoundAlternativeLocation = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this->GetWorld(), Location, NewLocation, 1000.0f);

		if (bFoundAlternativeLocation)
		{
			MoveBotToLocation(NewLocation);
		}

		//UE_LOG(LogTemp, Error, TEXT("[ARobotController] Failed to move robot. Target: %s. bFoundAlternative: %d"), *Location.ToString(), bFoundAlternativeLocation);
	}
}
