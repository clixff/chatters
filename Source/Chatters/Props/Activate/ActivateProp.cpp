// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivateProp.h"
#include "../../Core/ChattersGameSession.h"

// Sets default values
AActivateProp::AActivateProp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AActivateProp::BeginPlay()
{
	Super::BeginPlay();
	
	auto* GameSession = UChattersGameSession::Get();

	if (GameSession)
	{
		GameSession->PropsToActivate.Add(this);
	}
}

// Called every frame
void AActivateProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AActivateProp::Activate()
{
	bActivated = true;
}

void AActivateProp::Deactivate()
{
	bActivated = false;
}

