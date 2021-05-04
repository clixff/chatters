// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Field/FieldSystemActor.h"
#include "Components/BoxComponent.h"
#include "Field/FieldSystemObjects.h"
#include "GeometryCollection/GeometryCollectionSimulationTypes.h"
#include "Components/StaticMeshComponent.h"
#include "AnchorField.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API AAnchorField : public AFieldSystemActor
{
	GENERATED_BODY()
public:
	AAnchorField();
	~AAnchorField();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UBoxComponent* BoxComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UStaticMeshComponent* Cube = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UCullingField* CullingField = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UBoxFalloff* BoxFalloff = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UUniformInteger* UniformInteger = nullptr;

	UPROPERTY(EditDefaultsOnly)
		EObjectStateTypeEnum Type;

	UFUNCTION(BlueprintCallable)
		void SetFieldCommand(bool bEnabled = true);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
