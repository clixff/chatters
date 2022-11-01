// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Equipment/Weapon/FirearmWeaponItem.h"
#include "NiagaraSystem.h"
#include "Animation/AnimSequence.h"
#include "DestructibleComponent.h"
#include "../../../Misc/Misc.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Robot.generated.h"

class ABot;

USTRUCT(BlueprintType)
struct FRobotMaterials
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		TArray<UMaterialInterface*> Materials;

	UPROPERTY(EditAnywhere)
		UMaterialInterface* DestructedMaterial;
};

UCLASS()
class CHATTERS_API ARobot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARobot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveToNewLocation(FVector NewLocation);
	
	void StopMovement();

	void OnDead();

	void OnRespawn();

	UPROPERTY(EditAnywhere)
		UFirearmWeaponItem* WeaponClass = nullptr;

	UPROPERTY(EditAnywhere)
		FTransform CharacterTransform;

	UPROPERTY()
		ABot* BotOwner = nullptr;

	UPROPERTY(EditAnywhere)
		FVector NicknameOffset = FVector(0.0f, 0.0f, 210.0f);

	UPROPERTY(EditAnywhere)
		UAnimSequence* BotAnimation = nullptr;
public:

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		UParticleSystem* ParticleSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		FVector ParticleScale = FVector(1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		USoundBase* ExplodingSound = nullptr;

public:
	UPROPERTY(EditDefaultsOnly)
		UNiagaraSystem* DamageNiagaraParticle = nullptr;
public:
	UPROPERTY(VisibleAnywhere)
		UDestructibleComponent* DestructibleComponent = nullptr;
public:
	UPROPERTY(EditAnywhere)
		FManualTimer DestructibleTimer = FManualTimer(25.0f);

	bool bDestroyed = false;

	UPROPERTY(EditAnywhere)
		float ExplosionForce = 5000.0f;

	UPROPERTY(EditAnywhere)
		FVector ExplosionVector = FVector(0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere)
		float ExplosionDamage = 1;

	UPROPERTY(EditAnywhere)
		TArray<USceneComponent*> GunPoints;

	FVector GetGunPosition();

	UPROPERTY(EditAnywhere)
		float RunSpeed = 600.0f;

	UPROPERTY()
		FVector LastDamageLocation;

	void OnNewRound(EBotTeam BotTeam);
public:
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
		TArray<FRobotMaterials> RobotMaterials;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
		TArray<FRobotMaterials> BlueTeamMaterials;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
		TArray<FRobotMaterials> RedTeamMaterials;
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void PlayFootstepSound(const FVector& Location, EPhysicalSurface Surface);

	UFUNCTION(BlueprintCallable)
		void OnFootstep();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UCurveFloat* HitBoneRotationCurve = nullptr;
};
