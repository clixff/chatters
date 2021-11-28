// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponInstance.h"
#include "../FirearmWeaponItem.h"
#include "Engine/StaticMeshActor.h"
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

	UPROPERTY()
		AStaticMeshActor* ProjectileMeshActor = nullptr;

	FManualTimer ProjectileMeshRespawnTimer = FManualTimer(0.5f);

	void SetProjectileMeshVisibility(bool bVisible);

	UPROPERTY()
		USkeletalMeshComponent* BowstringComponent = nullptr;

	UPROPERTY()
		UBowstringAnimation* BowstringAnimInstance = nullptr;

	FManualTimer BowstringDetachTimer = FManualTimer(0.5f);
	FManualTimer BowstringAttachTimer = FManualTimer(0.5f);

	bool bShouldAttachBowstring = true;
private:
	void StartReloading();

	bool bSpawnedReloadingParticle = false;
};
