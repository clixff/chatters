// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RobotController.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API ARobotController : public AAIController
{
	GENERATED_BODY()
public:
	void MoveToNewLocation(FVector NewLocation);
};
