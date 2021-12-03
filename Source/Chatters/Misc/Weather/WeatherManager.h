// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/PostProcessVolume.h"
#include "WeatherManager.generated.h"

UCLASS()
class CHATTERS_API AWeatherManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeatherManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float Time = 0.0f;

	bool bActivated = false;

	void Activate();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* PostProccessBaseMaterial = nullptr;

	UPROPERTY()
		UMaterialInstanceDynamic* PostProccessMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		APostProcessVolume* PostProcessVolume = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
		float SecondsBeforeActivatingFog = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
		float FogInterpolationTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
		float ActivationChance = 1.0f;
};
