// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BotController.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API ABotController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABotController();
	~ABotController();

public:
	void MoveToLocation(FVector Location);
};
