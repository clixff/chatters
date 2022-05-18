// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimalController.h"
#include "NavigationSystem.h"


bool AAnimalController::MoveToVector(FVector Location)
{
	auto* PawnObject = this->GetPawn();
	FVector PawnLocation = FVector(0.0f);

	if (PawnObject)
	{
		PawnLocation = PawnObject->GetActorLocation();
	}

	auto MoveBotToLocation = [this](FVector LocationToMove)
	{
		return this->MoveToLocation(LocationToMove, 10.0f, false, true, true, true);
	};

	auto RequestResult = MoveBotToLocation(Location);

	return (RequestResult != EPathFollowingRequestResult::Type::Failed);
}

void AAnimalController::MoveToRandomLocation()
{
	bMovingToRandomLocation = true;

	for (int32 i = 0; i < 10; i++)
	{
		FVector OutLocation;
		bool bFound = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this->GetWorld(), GetPawn()->GetActorLocation(), OutLocation, 5000.0f);

		if (bFound)
		{
			bool bMoving = MoveToVector(OutLocation);

			if (bMoving)
			{
				bMovingToRandomLocation = true;
				TargetLocation = OutLocation;
				TimerWait.Reset();
				bWaiting = false;
				return;
			}
		}
	}
}

void AAnimalController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMovingToRandomLocation)
	{
		float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), TargetLocation);

		if (DistToTarget <= 200.0f)
		{
			TimerWait.Max = FMath::RandRange(TimeToWaiBetweenMoves.GetLowerBoundValue(), TimeToWaiBetweenMoves.GetUpperBoundValue());
			TimerWait.Current = 0.0f;
			bWaiting = true;
			StopMovement();
			bMovingToRandomLocation = false;
		}
	}

	if (bWaiting)
	{
		TimerWait.Add(DeltaTime);

		if (TimerWait.IsEnded())
		{
			bWaiting = false;
			MoveToRandomLocation();
		}
	}
}
