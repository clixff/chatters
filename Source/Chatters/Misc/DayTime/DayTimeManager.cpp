// Fill out your copyright notice in the Description page of Project Settings.


#include "DayTimeManager.h"
#include "../../Core/ChattersGameSession.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "../../Props/StreetLight.h"
#include "Components/SkyLightComponent.h"

// Sets default values
ADayTimeManager::ADayTimeManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ADayTimeManager::BeginPlay()
{
	Super::BeginPlay();
	
	auto* GameSession = UChattersGameSession::Get();
	if (GameSession)
	{
		GameSession->DayTimeManagerRef = this;
		TimeOfDay = GameSession->TimeOfDay;
	}
}

// Called every frame
void ADayTimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADayTimeManager::SetSunRotationAtDay(int32 RoundIndex)
{
	if (MoonActor)
	{
		MoonActor->Destroy();
		MoonActor = nullptr;
	}

	for (auto* Actor : NightStreetLights)
	{
		Actor->Destroy();
	}

	NightStreetLights.Empty();

	int32 MaxRounds = DaySunRotations.Num();
	if (!MaxRounds)
	{
		return;
	}

	if (RoundIndex == -1)
	{
		RoundIndex = FMath::RandRange(0, MaxRounds - 1);
	}

	int32 Index = RoundIndex % MaxRounds;

	if (!Light)
	{
		return;
	}

	FRotator Rotation = Light->GetActorRotation();
	Rotation.Pitch = DaySunRotations[Index];
	Rotation.Yaw = FMath::RandRange(0.0f, 360.0f);

	Light->SetActorRotation(Rotation);

	if (SkyLight)
	{
		auto* LightComponent = SkyLight->GetLightComponent();
		if (LightComponent)
		{
			LightComponent->RecaptureSky();
		}
	}
}

void ADayTimeManager::SetNightTime()
{
	bIsNight = true;
	if (Light)
	{
		FRotator Rotation = Light->GetActorRotation();
		Rotation.Pitch = NightSunRotation;
		Light->SetActorRotation(Rotation);
		Light->GetLightComponent()->SetIntensity(NightLightIntensity);
	}

	if (PostProcessVolume)
	{
		auto& PPSettings = PostProcessVolume->Settings;
		PPSettings.AutoExposureMaxBrightness = PPSettings.AutoExposureMinBrightness = NightBrightness;

		PPSettings.AmbientCubemapIntensity = NightAmbientScale;
	}

	if (Fog)
	{
		Fog->GetComponent()->SetFogInscatteringColor(NightFogColor);
	}


	if (SkyLight)
	{
		auto* LightComponent = SkyLight->GetLightComponent();
		if (LightComponent)
		{
			LightComponent->SourceType = ESkyLightSourceType::SLS_CapturedScene;
			LightComponent->RecaptureSky();
		}
	}

	for (auto* StreetLightActor : NightStreetLights)
	{
		auto* StreetLight = Cast<AStreetLight>(StreetLightActor);
		if (!StreetLight)
		{
			continue;
		}

		if (!StreetLight->StaticMesh || !StreetLight->StaticMesh->GetStaticMesh())
		{
			continue;
		}

		auto* StaticMesh = StreetLight->StaticMesh->GetStaticMesh();

		FString MeshName = StaticMesh->GetName();

		UHierarchicalInstancedStaticMeshComponent* HISM = nullptr;

		if (StreetLightHISMCs.Contains(MeshName))
		{
			HISM = StreetLightHISMCs[MeshName];
		}
		else
		{
			FString ComponentName = FString::Printf(TEXT("HISM_%s"), *MeshName);
			HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, *ComponentName);
			StreetLightHISMCs.Add(MeshName, HISM);
			HISM->SetStaticMesh(StaticMesh);

			HISM->SetCanEverAffectNavigation(StreetLight->StaticMesh->CanEverAffectNavigation());
			HISM->SetCastShadow(false);
			HISM->SetMobility(EComponentMobility::Static);
			HISM->AttachTo(GetRootComponent(), NAME_None, EAttachLocation::SnapToTarget);
			HISM->RegisterComponent();
			AddInstanceComponent(HISM);
		}

		if (HISM)
		{
			StreetLight->StaticMesh->SetVisibility(false);
			StreetLight->StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			FTransform LightTransform = StreetLight->StaticMesh->GetComponentTransform();
			HISM->AddInstance(LightTransform);
		}
	}
}

void ADayTimeManager::SetupZombieMode()
{
	if (PostProcessVolume)
	{
		if (ZombiePostProcessMaterial)
		{
			ZombiePostProcessMaterialInstance = UMaterialInstanceDynamic::Create(ZombiePostProcessMaterial, this);

			FLinearColor FogColor = bIsNight ? ZombieFogColorNight : ZombieFogColorDay;
			float FogScale = bIsNight ? ZombieFogColorScaleNight : ZombieFogColorScaleDay;

			ZombiePostProcessMaterialInstance->SetVectorParameterValue(TEXT("Color"), FogColor);
			ZombiePostProcessMaterialInstance->SetScalarParameterValue(TEXT("FogScale"), FogScale);

			auto& PPSettings = PostProcessVolume->Settings;

			FWeightedBlendable Blendable;

			Blendable.Object = ZombiePostProcessMaterialInstance;
			Blendable.Weight = 1.0f;
		
			//PPSettings.WeightedBlendables.Array.Add(Blendable);

			PPSettings.WhiteTint = ZombieTint;
			PPSettings.bOverride_WhiteTint = true;

			PPSettings.ColorSaturation = FVector4(FVector(ZombieSaturation));
			PPSettings.ColorContrast = FVector4(FVector(ZombieContrast));
			PPSettings.ColorGamma = FVector4(FVector(bIsNight ? ZombieGammaNight : ZombieGamma));
			PPSettings.ColorGain = FVector4(FVector(ZombieGain));

			PPSettings.bOverride_ColorSaturation = true;
			PPSettings.bOverride_ColorContrast = true;
			PPSettings.bOverride_ColorGamma = true;
			PPSettings.bOverride_ColorGain = true;

			if (Fog)
			{
				auto* FogComponent = Fog->GetComponent();

				if (FogComponent)
				{
					FogComponent->SetFogDensity(ZombieFogDensity);
					FogComponent->SetFogHeightFalloff(ZombieFogHeightFalloff);

					FogComponent->SecondFogData.FogDensity = ZombieFogSecondDensity;
					FogComponent->SecondFogData.FogHeightFalloff = ZombieFogSecondHeightFalloff;
					FogComponent->SecondFogData.FogHeightOffset = ZombieFogSecondHeightOffset;

					FogComponent->SetFogInscatteringColor(FogColor);
				}
			}
		}
	}
}

