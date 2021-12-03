// Fill out your copyright notice in the Description page of Project Settings.


#include "WeatherManager.h"
#include "../../Core/ChattersGameSession.h"

// Sets default values
AWeatherManager::AWeatherManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWeatherManager::BeginPlay()
{
	Super::BeginPlay();
	
	auto* GameSession = UChattersGameSession::Singleton;

	if (GameSession)
	{
		GameSession->WeatherManagerRef = this;
	}
}

// Called every frame
void AWeatherManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bActivated)
	{
		return;
	}

	this->Time += DeltaTime;

	float FogMinTime = SecondsBeforeActivatingFog;
	float FogMaxTime = FogMinTime + FogInterpolationTime;

	float ClampedTime = FMath::Clamp(Time, FogMinTime, FogMaxTime);

	float FogTimeScale = (ClampedTime - FogMinTime) / (FogMaxTime - FogMinTime);
	FogTimeScale = FMath::Clamp(FogTimeScale, 0.0f, 1.0f);

	if (this->PostProccessMaterial)
	{
		this->PostProccessMaterial->SetScalarParameterValue(TEXT("FogScale"), FogTimeScale);
	}

	if (ClampedTime >= FogMaxTime)
	{
		bActivated = false;
	}

}

void AWeatherManager::Activate()
{
	ActivationChance = FMath::Clamp(ActivationChance, 0.0f, 1.0f);

	int32 ActivationChanceMax = float(1.0f / ActivationChance);

	int32 ActivationChanceRand = FMath::RandRange(1, ActivationChanceMax);

	if (ActivationChanceRand != 1)
	{
		return;
	}

	if (!this->PostProccessBaseMaterial || !this->PostProcessVolume)
	{
		return;
	}

	this->bActivated = true;

	this->PostProccessMaterial = UMaterialInstanceDynamic::Create(this->PostProccessBaseMaterial, this);

	FWeightedBlendable Material;

	Material.Object = this->PostProccessMaterial;
	Material.Weight = 1.0f;

	this->PostProcessVolume->Settings.WeightedBlendables.Array.Add(Material);
}

