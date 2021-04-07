// Fill out your copyright notice in the Description page of Project Settings.


#include "BotController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

ABotController::ABotController()
{

}

ABotController::~ABotController()
{

}

void ABotController::MoveToLocation(FVector Location)
{
	auto* PawnObject = this->GetPawn();
	FVector PawnLocation = FVector(0.0f);
	
	if (PawnObject)
	{
		PawnLocation = PawnObject->GetActorLocation();
	}

	float Distance = FVector::Dist(PawnLocation, Location);

	UE_LOG(LogTemp, Display, TEXT("[ABotController] Moving bot from %s to %s. Distance: %f m"), *(PawnLocation.ToString()), *(Location.ToString()), Distance);

	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Location);
}