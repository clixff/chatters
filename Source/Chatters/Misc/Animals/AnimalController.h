// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
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

protected:
	virtual void Tick(float DeltaTime) override;
};
