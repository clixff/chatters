// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "DayTimeManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Day,
	Night
};

UCLASS()
class CHATTERS_API ADayTimeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADayTimeManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		TArray<float> DaySunRotations;

	UPROPERTY()
		ETimeOfDay TimeOfDay = ETimeOfDay::Day;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		APostProcessVolume* PostProcessVolume = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ADirectionalLight* Light = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AExponentialHeightFog* Fog = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ASkyLight* SkyLight = nullptr;

public:
	void SetSunRotationAtDay(int32 RoundIndex);

	void SetNightTime();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Night")
		float NightSunRotation = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Night")
		float NightLightIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Night")
		float NightBrightness = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Night")
		float NightAmbientScale = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Night")
		FLinearColor NightFogColor = FLinearColor(0.026f, 0.037f, 0.056f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Night")
		TArray<AActor*> NightStreetLights;

	UPROPERTY()
		TMap<FString, UHierarchicalInstancedStaticMeshComponent*> StreetLightHISMCs;

	UPROPERTY(EditAnywhere)
		AActor* MoonActor = nullptr;

	bool bIsNight = false;

public:
	UPROPERTY(EditAnywhere, Category = "Zombie")
		UMaterialInterface* ZombiePostProcessMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Zombie")
		UMaterialInstanceDynamic* ZombiePostProcessMaterialInstance = nullptr;

	UPROPERTY(EditAnywhere, Category = "Zombie|Fog")
		FLinearColor ZombieFogColorDay = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Zombie|Fog")
		FLinearColor ZombieFogColorNight = FLinearColor(0.01f, 0.01f, 0.005f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Zombie")
		float ZombieFogColorScaleDay = 0.9f;

	UPROPERTY(EditAnywhere, Category = "Zombie")
		float ZombieFogColorScaleNight = 0.9f;

	UPROPERTY(EditAnywhere, Category = "Zombie")
		float ZombieTint = -0.25f;

	UPROPERTY(EditAnywhere, Category = "Zombie")
		float ZombieSaturation = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Zombie")
		float ZombieContrast = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Zombie")
		float ZombieGamma = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Zombie")
		float ZombieGammaNight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Zombie")
		float ZombieGain = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Zombie|Fog")
		float ZombieFogDensity = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Zombie|Fog")
		float ZombieFogHeightFalloff = 0.001f;

	UPROPERTY(EditAnywhere, Category = "Zombie|Fog")
		float ZombieFogSecondDensity = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Zombie|Fog")
		float ZombieFogSecondHeightFalloff = 0.001f;

	UPROPERTY(EditAnywhere, Category = "Zombie|Fog")
		float ZombieFogSecondHeightOffset = 125000.0f;

	void SetupZombieMode();
};
