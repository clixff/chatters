// Fill out your copyright notice in the Description page of Project Settings.


#include "BotSpawnPoint.h"
#include "../Core/ChattersGameSession.h"

// Sets default values
ABotSpawnPoint::ABotSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	this->SetRootComponent(this->CapsuleComponent);
	this->CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	this->ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	this->ArrowComponent->SetupAttachment(this->CapsuleComponent);
}

ABotSpawnPoint::~ABotSpawnPoint()
{

}

// Called when the game starts or when spawned
void ABotSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp, Display, TEXT("[ABotSpawnPoint] Bot spawn point spawned"));

	auto* GameSession = UChattersGameSession::Get();

	if (GameSession)
	{
		GameSession->BotSpawnPoints.Add(this);
	}
}

// Called every frame
void ABotSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FRotator ABotSpawnPoint::GetRotation()
{
	FRotator Rotation = this->GetActorRotation();

	if (this->bRandomYawRotation)
	{
		Rotation.Yaw = FMath::RandRange(0.0f, 360.0f);
	}

	return Rotation;
}

