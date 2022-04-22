// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActivateProp.generated.h"

UCLASS()
class CHATTERS_API AActivateProp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AActivateProp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Activate();
	virtual void Deactivate();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bActivateEveryRound = false;


	bool bActivated = false;
};
