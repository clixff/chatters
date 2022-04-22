// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../ActivateProp.h"
#include "../../../Misc/Misc.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "ScifiWalker.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API AScifiWalker : public AActivateProp
{
	GENERATED_BODY()
public:
	AScifiWalker();

	virtual void Activate() override;

	virtual void Tick(float DeltaTime) override;

	void ProjectileTick(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FManualTimer Timer = FManualTimer(15.0f);

	void Attack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ProjectileSpeed = 10000.0f;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* MainComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UArrowComponent* ProjectileOutPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundBase* FireSound;

	UPROPERTY(EditAnywhere, Category = "Explosion")
		UParticleSystem* ExplosionParticle = nullptr;

	UPROPERTY(EditAnywhere, Category = "Explosion")
		FVector ExplosionParticleScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
		USoundBase* ExplosionSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Explosion")
		float ExplosionRadius = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Explosion")
		float ImpulseForce = 25000.0f;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> ProjectileActorClass;

	UPROPERTY(VisibleAnywhere)
		AActor* ProjectileActor;
private:
	FVector ProjectileStartLocation;
	FVector TargetLocation;

	bool bProjectileMoving = false;

	FManualTimer ProjectileTimer = FManualTimer(5.0f);
};
