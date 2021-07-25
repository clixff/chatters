// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponInstance.h"
#include "../FirearmWeaponItem.h"
#include "FirearmWeaponInstance.generated.h"

UENUM(BlueprintType)
enum class EFirearmPhase : uint8
{
	IDLE,
	Shooting,
	Reloading
};

/**
 * 
 */
UCLASS()
class CHATTERS_API UFirearmWeaponInstance : public UWeaponInstance
{
	GENERATED_BODY()
public:
	UFirearmWeaponInstance();
	~UFirearmWeaponInstance();

	UFirearmWeaponItem* GetFirearmRef();

	UPROPERTY(VisibleAnywhere)
		int32 NumberOfBullets = 0;

	virtual void Tick(float DeltaTime) override;

	virtual void Init() override;

	UPROPERTY()
		EFirearmPhase Phase = EFirearmPhase::IDLE;

	bool CanShoot();

	void OnShoot();

	FLinearColor TraceColor;
private:
	void StartReloading();

	bool bSpawnedReloadingParticle = false;
};
