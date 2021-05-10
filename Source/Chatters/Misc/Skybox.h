// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/PostProcessVolume.h"
#include "Components/SkyLightComponent.h"
#include "Engine/SkyLight.h"
#include "Engine/DirectionalLight.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Skybox.generated.h"

UCLASS()
class CHATTERS_API ASkybox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASkybox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Common")
		USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Common")
		UStaticMeshComponent* SphereComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common")
		FLinearColor ZenithColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common")
		FLinearColor HorizonColor;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		ADirectionalLight* DirectionalLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		APostProcessVolume* GlobalPostProcess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		ASkyLight* SkyLight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common")
		UMaterialInterface* BaseMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Common")
		UMaterialInstanceDynamic* DynamicMaterial;
protected:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void SetDynamicMaterial();

public:
	virtual void OnConstruction(const FTransform& Transform) override;

	void UpdateDynamicMaterial();
};
