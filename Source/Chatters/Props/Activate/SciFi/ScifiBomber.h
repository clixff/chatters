// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../ActivateProp.h"
#include "../../../Misc/Misc.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "ScifiBomber.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API AScifiBomber : public AActivateProp
{
	GENERATED_BODY()
public:
	AScifiBomber();

	virtual void Activate() override;

	virtual void Deactivate() override;

	virtual void Tick(float DeltaTime) override;

	void ProjectileTick(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FFloatRange YRange = FFloatRange(-14264.0f, 6543.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FFloatRange XRange = FFloatRange(-15399.0f, 16280.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FlyOffset = 65000.0f;

	void Attack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ProjectileSpeed = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FlySpeed = 150000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FManualTimer FlyTimer = FManualTimer(15.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FManualTimer ProjectileTimer = FManualTimer(15.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FManualTimer TimeoutTimer = FManualTimer(5.0f);

	bool bFlying = false;

	void FlyTick(float DeltaTime);

	void StartFlying();
	void StopFlying();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* MainComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UArrowComponent* ProjectileOutPosition;

	UPROPERTY(EditAnywhere, Category = "Explosion")
		UParticleSystem* ExplosionParticle = nullptr;

	UPROPERTY(EditAnywhere, Category = "Explosion")
		FVector ExplosionParticleScale = FVector(3.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
		USoundBase* ExplosionSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAudioComponent* EngineSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Explosion")
		float ExplosionRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Explosion")
		float ImpulseForce = 50000.0f;

	UPROPERTY(EditAnywhere)
		float FlyingHeight = 10000.0f;

	UPROPERTY(EditAnywhere)
		float ProjectileOffset = 10000.0f;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> ProjectileActorClass;

	UPROPERTY(VisibleAnywhere)
		AActor* ProjectileActor;
private:
	FVector2D StartLocation;

	FVector2D EndLocation;

	FVector2D AttackLocation;

	bool bAttacked = false;
private:
	FVector ProjectileStartLocation;
	FVector ProjectileEndLocation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
