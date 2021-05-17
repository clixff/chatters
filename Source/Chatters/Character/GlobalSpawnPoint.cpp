// Fill out your copyright notice in the Description page of Project Settings.


#include "GlobalSpawnPoint.h"

// Sets default values
AGlobalSpawnPoint::AGlobalSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGlobalSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGlobalSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

