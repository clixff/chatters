// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Props/ExplodingBarrel.h"
#include "../Character/Bot.h"
#include "../Character/Equipment/Weapon/Instances/FirearmWeaponInstance.h"
#include "NiagaraComponent.h"
#include "FirearmProjectile.generated.h"

enum class ETraceLengthAction : uint8
{
	Reduce,
	Increase,
	Ignore
};

UCLASS()
class CHATTERS_API AFirearmProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFirearmProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	/** Meters in second */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float Speed = 10.0f;

	FVector StartLocation;
	FVector EndLocation;

	float Time = 0.0f;
	float MaxTime = 0.0f;

	void Init(FVector InitStartLocation, FVector InitEndLocation, FBulletHitResult HitResult, UFirearmWeaponInstance* FirearmInstanceRef, FVector BotForwardVector);

	void OnEnd();

	ABot* BotCauser = nullptr;

	float Distance = 0.0f;

	bool bActive = false;

	FBulletHitResult BulletHitResult;
	FVector CauserForwardVector;
	UFirearmWeaponItem* FirearmRef = nullptr;
	UFirearmWeaponInstance* FirearmInstance = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintREadOnly)
		UNiagaraComponent* Trace = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float TraceMaxLength = 700.0f;

	void SetTraceLocation();

	float DistanceScale = 0.0f;

	ETraceLengthAction TraceLengthAction = ETraceLengthAction::Increase;

	UPROPERTY(EditDEfaultsOnly)
		float TraceLengthSpeed = 1000.0f;

	float TraceLength = 0.0f;

	void UpdateTraceLength();

	void UpdateTraceOpacity();

	float Opacity = 0.0f;

	UPROPERTY(EditDefaultsOnly)
		float TraceLengthMaxOpacity = 500.0f;

	static uint32 TotalNumberOfProjectiles;

	static FName GenerateName();
};
