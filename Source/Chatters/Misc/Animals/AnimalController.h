// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "../Misc.h"
#include "AnimalController.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API AAnimalController : public AAIController
{
	GENERATED_BODY()
public:
	bool MoveToVector(FVector Location);

	void MoveToRandomLocation();

	bool bMovingToRandomLocation = true;

	FVector TargetLocation;

	UPROPERTY(EditAnywhere)
		FFloatRange TimeToWaiBetweenMoves = FFloatRange(1.0f, 2.5f);

	UPROPERTY()
		FManualTimer TimerWait = FManualTimer(1.0f);

protected:
	virtual void Tick(float DeltaTime) override;

	bool bWaiting = false;
};
