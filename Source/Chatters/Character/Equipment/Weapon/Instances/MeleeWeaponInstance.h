// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponInstance.h"
#include "../MeleeWeaponItem.h"
#include "MeleeWeaponInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UMeleeWeaponInstance : public UWeaponInstance
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTIme) override;

	UMeleeWeaponItem* GetMeleeRef();

	EMeleePhase Phase = EMeleePhase::IDLE;

	void OnHit();
	
	UPROPERTY()
		TArray<AActor*> BotsHit;

	bool CanHit();

	bool bCollisionEnabled = false;

	void SetCollisionEnabled(bool bEnabled);

	float TimeToEnableCollision = 0.0f;
	float TimeToDisableCollision = 1.0f;
};
