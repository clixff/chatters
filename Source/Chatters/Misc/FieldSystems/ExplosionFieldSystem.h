// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Field/FieldSystemActor.h"
#include "Field/FieldSystemObjects.h"
#include "Components/SphereComponent.h"
#include "ExplosionFieldSystem.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API AExplosionFieldSystem : public AFieldSystemActor
{
	GENERATED_BODY()
public:
	AExplosionFieldSystem();
	~AExplosionFieldSystem();

	void Explode();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USphereComponent* SphereComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UOperatorField* OperatorField = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		URadialFalloff* ForceRadialFalloff = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		URadialVector* ForceRadialVector = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float StrainMagnitude;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float ForceMagnitude;
};
