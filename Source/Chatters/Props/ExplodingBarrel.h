// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundBase.h"
#include "../Misc/FieldSystems/ExplosionFieldSystem.h"
#include "Components/SphereComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "ExplodingBarrel.generated.h"

class ABot;

UCLASS()
class CHATTERS_API AExplodingBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplodingBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
		bool bCanExplode = true;

	UPROPERTY(EditDefaultsOnly)
		float Radius = 500.0f;

	UPROPERTY(EditDefaultsOnly)
		int32 MaxDamage = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Particle")
		UParticleSystem* ParticleSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
		FVector ParticleScale = FVector(1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		USoundBase* ExplodingSound = nullptr;

	UPROPERTY(VisibleAnywhere)
		UGeometryCollectionComponent* Mesh = nullptr;

	UPROPERTY(VisibleAnywhere)
		USphereComponent* SphereComponent = nullptr;

	void Explode(ABot* BotCauser);

	TArray<ABot*> GetBotsInRadius();

	UPROPERTY(VisibleAnywhere, Category= "FieldSystem" )
		AExplosionFieldSystem* FieldSystemActor = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "FieldSystem")
		TSubclassOf<AExplosionFieldSystem> FieldSystemSubclass;

#if WITH_EDITOR  
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
